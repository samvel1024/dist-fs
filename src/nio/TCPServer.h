//
// Created by Samvel Abrahamyan on 2019-05-26.
//

#ifndef DISTFS_TCPSERVER_H
#define DISTFS_TCPSERVER_H

#include "Subscriber.h"

class TCPSessionFactory;

class TCPServer : public Subscriber {
 public:
  typedef std::function<std::shared_ptr<Subscriber>(Poll &p, TCPServer &server, sockaddr_in client)> SessionFactory;
 private:
  int port;
  SessionFactory client_factory;
 public:

  void on_input(Poll &p) override;

  void on_output(Poll &p) override;

  explicit TCPServer(std::string nm, SessionFactory factory, int port = 0);
  int get_port();
  virtual ~TCPServer();
};

#endif //DISTFS_TCPSERVER_H
