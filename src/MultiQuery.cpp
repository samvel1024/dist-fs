#include <utility>
#include "nio/Error.h"
#include "MultiQuery.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Dto.h"

std::string buffer(100000, '\0');

template<class REQ, class RES>
MultiQuery<REQ, RES>::MultiQuery(REQ &req, uint16_t port, std::string addr, int timeout) :
    Subscriber("MultiQuery"),
    req(std::move(req)),
    addr(addr), timeout(timeout) {

  struct sockaddr_in remote_address{};

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
  std::cout << "MultiQuery: sent " << this->req << std::endl;
  set_expected(POLLIN);
  p.notify_subscriber_changed(*this);
  if (this->timeout > 0) {
    p.subscribe_alarm(std::make_shared<Alarm>(this->timeout, [&] {
      if (this->done)
        this->done();
      p.unsubscribe(*this);
    }));
  } else {
    p.unsubscribe(*this);
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
  RES dto;
  try {
    dto = dto::unmarshall<RES>(data);
  } catch (Error &e) {
    if (this->error)
      this->error();
    return;
  }
  std::cout << "MultiQuery: received " << dto << std::endl;
  if (dto.header.cmd_seq != this->req.header.cmd_seq) {
    char *s = inet_ntoa(remote.sin_addr);
    uint16_t port = ntohs(remote.sin_port);
    std::cout << "[PCKG ERROR] Skipping invalid package from " << s << ":" << port << ". " << std::endl;
  } else {
    if (this->callback)
      this->callback(dto, remote);
  }
}

template<class REQ, class RES>
void MultiQuery<REQ, RES>::on_error(Poll &p, int event) {
  this->error();
  Subscriber::on_error(p, event);
}

template<class REQ, class RES>
void MultiQuery<REQ, RES>::when_response(MultiQuery::OnResponse resp) {
  this->callback = std::move(resp);
}
template<class REQ, class RES>
void MultiQuery<REQ, RES>::when_error(MultiQuery::OnError err) {
  this->error = std::move(err);
}
template<class REQ, class RES>
void MultiQuery<REQ, RES>::when_timeout(MultiQuery::OnTimeout t) {
  this->done = std::move(t);
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