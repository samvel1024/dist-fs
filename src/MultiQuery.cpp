#include <utility>
#include "nio/Error.h"
#include "MultiQuery.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Dto.h"

std::string buffer(100000, '\0');

template<class REQ, class RES>
MultiQuery<REQ, RES>::MultiQuery(uint16_t port, std::string addr, int tm) :
    Subscriber("MultiQuery"), req(), addr(addr), timeout(tm) {

  struct sockaddr_in remote_address{}, local_address{};

  int sock = no_err(socket(AF_INET, SOCK_DGRAM, 0), "Could not open udp socket");

  int optval = 1;
  no_err(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &optval, sizeof optval), "setsockopt SO_BROADCAST");
  optval = 4;
  no_err(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &optval, sizeof optval), "setscokopt TTL");

  remote_address.sin_family = AF_INET;
  remote_address.sin_port = htons(port);
  no_err(inet_aton(&addr[0], &remote_address.sin_addr), "inet_aton");
  remote = remote_address;
  set_fd(sock);
  set_expected(POLLOUT);
}

template<class REQ, class RES>
void MultiQuery<REQ, RES>::on_output(Poll &p) {
  auto data = dto::marshall(this->req);
  if (sendto(fd, &data[0], data.size(), 0, (struct sockaddr *) &remote, sizeof(remote)) != data.size()) {
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

template<class REQ, class RES>
void MultiQuery<REQ, RES>::on_input(Poll &p) {
  socklen_t remln = sizeof(remote);
  int read = recvfrom(fd, &buffer[0], buffer.size(), 0, (struct sockaddr *) &remote, &remln);
  if (read < 0) {
    throw Error("Invalid read udp packet");
  }
  std::string data = buffer.substr(0, read);
  auto dto = dto::unmarshall<RES>(data);
  if (dto.header.cmd_seq != this->req.header.cmd_seq) {
    this->error();
  } else {
    this->callback(dto, remote);
  }
  if (this->timeout == 0) {
    this->done();
  }
}

template<class REQ, class RES>
void MultiQuery<REQ, RES>::on_error(Poll &p, int event) {
  this->error();
  Subscriber::on_error(p, event);
}

template<typename REQ, typename RES>
void MultiQuery<REQ, RES>::execute(REQ &req, std::function<void(RES &, sockaddr_in)> callback,
                                   std::function<void(void)> error, std::function<void(void)> done) {
  this->req = std::move(req);
  this->callback = std::move(callback);
  this->error = std::move(error);
  this->done = std::move(done);
}

// Nasty hack to avoid writing all in header file
template
class MultiQuery<dto::Simple, dto::Complex>;

template
class MultiQuery<dto::Simple, dto::Simple>;

template
class MultiQuery<dto::Complex, dto::Complex>;

template
class MultiQuery<dto::Complex, dto::Simple>;