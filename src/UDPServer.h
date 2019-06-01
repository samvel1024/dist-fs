//
// Created by Samvel Abrahamyan on 2019-05-27.
//

#ifndef DISTFS_UDPSERVER_H
#define DISTFS_UDPSERVER_H

#include "nio/Subscriber.h"
#include "Dto.h"
#include "SharedDirectory.h"

class UDPServer : public Subscriber {
 private:
  static constexpr int buf_len = 2000;
  static constexpr int sockaddr_len = sizeof(sockaddr_in);
  struct sockaddr_in current_client;
  std::string buffer;
  std::shared_ptr<SharedDirectory> dir;
  const int timeout;
  const uint16_t port;
  const std::string mcast_addr;

  void on_dispatch(Poll &p, int bytes_read);

  void on_hello(Poll &p, dto::Simple &msg);

 public:

  void on_input(Poll &p) override;

  void on_output(Poll &p) override;

  ~UDPServer() override;

  explicit UDPServer(std::string name, std::string addr, uint16_t port, std::shared_ptr<SharedDirectory> shdir,
                     int timeout);

  void on_list(Poll &poll, dto::Simple &simple);
  void on_download(Poll &poll, dto::Simple simple);
};

#endif //DISTFS_UDPSERVER_H
