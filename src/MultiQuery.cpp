#include <utility>
#include "nio/Error.h"
#include "MultiQuery.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

std::string buffer(100000, '\0');

MultiQuery::MultiQuery(uint16_t port, std::string addr, int tm) :
	Subscriber("MultiQuery"), req(), addr(addr), timeout(tm) {

	struct sockaddr_in remote_address{}, local_address{};

	int sock = no_err(socket(AF_INET, SOCK_DGRAM, 0), "Could not open udp socket");

	int optval = 1;
	no_err(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &optval, sizeof optval), "setsockopt SO_BROADCAST");
	optval = 4;
	no_err(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &optval, sizeof optval), "setscokopt TTL");
	optval = 0;
	no_err(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (void *) &optval, sizeof optval), "setsockopt loop");

	local_address.sin_family = AF_INET;
	local_address.sin_addr.s_addr = htonl(INADDR_ANY);
	local_address.sin_port = htons(0);
	no_err(bind(sock, (struct sockaddr *) &local_address, sizeof local_address), "Bind");

	remote_address.sin_family = AF_INET;
	remote_address.sin_port = htons(port);
	no_err(inet_aton(&addr[0], &remote_address.sin_addr), "inet_aton");
	remote = remote_address;
	set_fd(sock);
	set_expected(POLLOUT);
}

void
MultiQuery::execute(const std::string &req_str, std::function<void(std::string &, sockaddr_in)> callback,
                    std::function<void()> error,
                    std::function<void()> done) {
	this->req = req_str;
	this->callback = std::move(callback);
	this->error = std::move(error);
	this->done = std::move(done);
}

void MultiQuery::on_output(Poll &p) {
	if (sendto(fd, &req[0], req.size(), 0, (struct sockaddr *) &remote, sizeof(remote)) != req.size()) {
		throw Error("Could not write");
	}
	set_expected(POLLIN);
	p.notify_subscriber_changed(*this);
	if (this->timeout > 0) {
		p.subscribe_alarm(std::make_shared<Alarm>(this->timeout, [&] {
			this->done();
			p.unsubscribe(*this);
		}));
	}
}

void MultiQuery::on_input(Poll &p) {
	socklen_t remln = sizeof(remote);
	int read = recvfrom(fd, &buffer[0], buffer.size(), 0, (struct sockaddr *) &remote, &remln);
	if (read < 0) {
		throw Error("Invalid read udp packet");
	}
	std::string data = buffer.substr(0, read);
	this->callback(data, remote);
	if (this->timeout == 0) {
		this->done();
	}
}

void MultiQuery::on_error(Poll &p, int event) {
	this->error();
	Subscriber::on_error(p, event);
}
