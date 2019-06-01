#include <utility>

//
// Created by Samvel Abrahamyan on 2019-05-26.
//

#include "TCPServer.h"
#include "Error.h"

void TCPServer::on_input(Poll &p) {
  int connected_fd;
  do {
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    connected_fd = accept(this->get_fd(), (struct sockaddr *) &client, &client_len);
    if (connected_fd > 0) {
      std::shared_ptr<Subscriber> session = client_factory(p, *this, client);
      session->set_fd(connected_fd);
      p.subscribe(session);
      std::cout << this->name << ": new connection" << std::endl;
    }
  } while (connected_fd > 0);
  if (connected_fd < 0 && errno != EWOULDBLOCK) {
    no_err(-1, "accept failed");
  }
}

void TCPServer::on_output(Poll &p) {

}

TCPServer::TCPServer(std::string nm, TCPServer::SessionFactory factory, int port) :
    Subscriber(std::move(nm)),
    client_factory(std::move(factory)) {
  struct sockaddr_in server_address{};
  server_address.sin_family = AF_INET; // IPv4
  server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
  server_address.sin_port = htons(port);
  int sock = no_err(socket(PF_INET, SOCK_STREAM, 0), "create socket"); // creating IPv4 TCP file_desc

  int yes = 1;
  no_err(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)), "setsockopt reuseaddr");
  no_err(bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)), "bind");
  // switch to listening (passive open)
  no_err(listen(sock, 5), "listen");
  set_fd(sock);
  set_expected(POLLIN);
  socklen_t len = sizeof(server_address);
  no_err(getsockname(sock, (struct sockaddr *) &server_address, &len), "getsockname");
  this->port = ntohs(server_address.sin_port);
}

int TCPServer::get_port() {
  return this->port;
}

TCPServer::~TCPServer() {

  std::cout << "Closed TCP server on port: " << get_port() << std::endl;

}

