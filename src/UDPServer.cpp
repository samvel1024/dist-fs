#include <utility>

//
// Created by Samvel Abrahamyan on 2019-05-27.
//

#include "UDPServer.h"
#include "Dto.h"
#include "nio/TCPServer.h"
#include "nio/Poll.h"
#include "nio/TCPServer.h"
#include "FileSendSession.h"
#include "FileReceiveSession.h"

namespace fs = boost::filesystem;
constexpr int MAX_PAYLOAD_SIZE = 400;

int connect_group(in_port_t port, const char *addr) {
  int sock = no_err(socket(AF_INET, SOCK_DGRAM, 0), "socket");
  u_int yes = 1;
  no_err(setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char *) &yes, sizeof(yes)), "reuse");
  //Connect to group
  struct ip_mreq ip_mreq{};
  ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  no_err(inet_aton(addr, &ip_mreq.imr_multiaddr) == 0, "inet_aton");
  no_err(setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &ip_mreq, sizeof ip_mreq), "setsockopt");
  //Bind to port
  struct sockaddr_in local_address{};
  local_address.sin_family = AF_INET;
  local_address.sin_addr.s_addr = htonl(INADDR_ANY);
  local_address.sin_port = htons(port);
  no_err(bind(sock, (struct sockaddr *) &local_address, sizeof local_address), "bind");
  no_err(fcntl(sock, F_SETFL, O_NONBLOCK), "set nonblock");

  return sock;
}

void UDPServer::on_input(Poll &p) {
  unsigned clientlen; /* byte size of client's address */
  clientlen = sizeof(current_client);
  int bytes_read = no_err(recvfrom(fd, &buffer[0], buffer.size(), 0,
                                   (struct sockaddr *) &current_client, &clientlen), "Error in recvfrom");
  route_request(p, bytes_read);
}

template<typename T>
T parse(std::string data) {
  T dto = dto::unmarshall<T>(data);
  std::cout << "UDPServer: received " << dto << std::endl;
  return dto;
}

template<typename T>
void UDPServer::respond(Poll &p, T dto) {
  std::cout << "UDPServer: responding " << dto << std::endl;
  auto resp_str = dto::marshall(dto);
  msg_queue.emplace(std::make_pair(std::move(resp_str), current_client));
  set_expected(POLLIN | POLLOUT);
  p.notify_subscriber_changed(*this);
}

void UDPServer::on_output(Poll &p) {
  if (msg_queue.empty()) {
    set_expected(POLLIN);
    p.notify_subscriber_changed(*this);
    return;
  }
  auto msg = msg_queue.front();
  msg_queue.pop();
  socklen_t len = sizeof(msg.second);
  if (msg.first.size() != sendto(fd, &msg.first[0], msg.first.size(), 0, (struct sockaddr *) &msg.second, len)) {
    std::cout << "UDPServer: error in writing to udp socket " << from_errno() << std::endl;
  }
  if (msg_queue.empty()) {
    set_expected(POLLIN);
    p.notify_subscriber_changed(*this);
  }
}

void UDPServer::route_request(Poll &p, int bytes_read) {
  std::string type_header = buffer.substr(0, dto::CMD_TYPE_LEN);
  std::string data = buffer.substr(0, bytes_read);
  try {
    switch (dto::from_header(type_header)) {
      case dto::HELLO_REQ : {
        auto dto = parse<dto::Simple>(data);
        on_hello(p, dto);
        break;
      }
      case dto::LIST_REQ : {
        auto dto = parse<dto::Simple>(data);
        on_list(p, dto);
        break;
      }
      case dto::DOWNLOAD_REQ: {
        auto dto = parse<dto::Simple>(data);
        on_download(p, dto);
        break;
      }
      case dto::UPLOAD_REQ: {
        auto dto = parse<dto::Complex>(data);
        on_upload(p, dto);
        break;
      }
      case dto::DEL_REQ: {
        auto dto = parse<dto::Simple>(data);
        on_delete(p, dto);
        break;
      }
      default : {
        throw Error("Illegal cmd type");
      }
    }
  } catch (Error &err) {
    char *s = inet_ntoa(current_client.sin_addr);
    uint16_t port = ntohs(current_client.sin_port);
    std::cout << "[PCKG ERROR] Skipping invalid package from " << s << ":" << port << ". " << err.what() << std::endl;
  }
}

void UDPServer::on_hello(Poll &p, dto::Simple &msg) {
  if (msg.payload.size() != 0) {
    throw Error("Hello should not have a payload");
  }
  std::string mcast = this->mcast_addr;
  auto resp = dto::create(msg.header.cmd_seq, "GOOD_DAY", mcast, dir->get_remaining_space());
  respond(p, resp);
}

void UDPServer::on_list(Poll &poll, dto::Simple &msg) {
  std::string query = std::move(msg.payload);
  auto file_list = dir->search_file(query);
  if (file_list.empty()) {
    return;
  }
  std::string payload;
  for (auto &file: file_list) {
    if (file.size() + payload.size() >= MAX_PAYLOAD_SIZE) {
      auto resp = dto::create(msg.header.cmd_seq, "MY_LIST", payload);
      payload.clear();
      respond(poll, resp);
    }
    if (!payload.empty()) {
      payload += '\n';
    }
    payload += file;
  }
  if (!payload.empty()) {
    auto resp = dto::create(msg.header.cmd_seq, "MY_LIST", payload);
    respond(poll, resp);
  }
}

void UDPServer::on_download(Poll &poll, dto::Simple &simple) {
  if (!this->dir->can_read_file(simple.payload)) {
    std::cout << "The file " << simple.payload << " does not exist" << std::endl;
    return;
  }
  fs::path fp = dir->path_in_dir(simple.payload);
  auto tcp = std::make_shared<TCPServer>(
      std::string("FileDownloadServer-") + simple.payload,
      FileSendSession::create_session_factory(fp));
  poll.subscribe(tcp);
  std::weak_ptr<TCPServer> server = tcp;
  auto alarm = std::make_shared<Alarm>(timeout * 1000, [server, &poll]() -> void {
    if (!server.expired())
      poll.unsubscribe(*server.lock());
  });
  auto resp = dto::create(simple.header.cmd_seq, "CONNECT_ME", simple.payload, (uint64_t) tcp->get_port());
  respond(poll, resp);
  std::cout << "UDPServer: created file download tcp server listening on port " << tcp->get_port() << std::endl;
}

void UDPServer::on_upload(Poll &poll, dto::Complex &complex) {
  std::string file_name = complex.payload;
  if (!dir->can_create_file(complex.header.param, file_name)) {
    auto dto = dto::create(complex.header.cmd_seq, "NO_WAY", file_name);
    respond(poll, dto);
    return;
  }
  dir->reserve_file(file_name, complex.header.param);
  std::shared_ptr<SharedDirectory> sdir = this->dir;

  auto tcp = std::make_shared<TCPServer>(
      std::string("FileUploadServer-") + file_name,
      FileReceiveSession::create_session_factory(dir->path_in_dir(file_name))
  );
  poll.subscribe(tcp);
  std::weak_ptr<TCPServer> server = tcp;
  auto alarm = std::make_shared<Alarm>(timeout * 1000, [server, &poll, file_name, sdir]() -> void {
    if (!server.expired()) {
      poll.unsubscribe(*server.lock());
      sdir->cancel_reserved_file(file_name);
    }
  });
  poll.subscribe_alarm(alarm);
  std::string nil;
  auto ok = dto::create(complex.header.cmd_seq, "CAN_ADD", nil, tcp->get_port());
  respond(poll, ok);
  std::cout << "UDPServer: created file upload tcp server listening on port " << tcp->get_port() << std::endl;
}

void UDPServer::on_delete(Poll &poll, dto::Simple &simple) {
  if (!dir->can_read_file(simple.payload)) {
    std::cout << "UDPServer: illegal filename" << std::endl;
    return;
  }
  dir->delete_file(simple.payload);
  std::cout << "UDPServer: deleted file " << simple.payload << std::endl;
}

UDPServer::~UDPServer() {
  struct ip_mreq ip_mreq;
  ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void *) &ip_mreq, sizeof ip_mreq) < 0) {
    std::cout << "Could not unsubcribe from multicast group" << std::endl;
  }
}
UDPServer::UDPServer(const std::string name, std::string addr, uint16_t port, std::shared_ptr<SharedDirectory> shdir,
                     int timeout)
    : Subscriber(name),
      buffer(buf_len, '\0'),
      dir(std::move(shdir)),
      timeout(timeout),
      port(port),
      mcast_addr(addr) {
  set_fd(connect_group(port, addr.c_str()));
  set_expected(POLLIN);
  std::cout << name << ": Listening on port " << port << std::endl;
}

