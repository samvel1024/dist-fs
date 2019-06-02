//
// Created by Samvel Abrahamyan on 2019-05-27.
//

#ifndef DISTFS_UDPSERVER_H
#define DISTFS_UDPSERVER_H

#include "nio/Subscriber.h"
#include "Dto.h"
#include "SharedDirectory.h"
#include <queue>

class UDPServer : public Subscriber {
 private:
  typedef std::pair<std::string, sockaddr_in> Msg;
  static constexpr int buf_len = 2000;
  static constexpr int sockaddr_len = sizeof(sockaddr_in);
  struct sockaddr_in current_client;
  std::string buffer;
  std::shared_ptr<SharedDirectory> dir;
  const int timeout;
  const uint16_t port;
  const std::string mcast_addr;
  std::queue<Msg> msg_queue;

  void route_request(Poll &p, int bytes_read);

  void on_hello(Poll &p, dto::Simple &msg);

 public:

  void on_input(Poll &p) override;

  ~UDPServer() override;

  explicit UDPServer(std::string name, std::string addr, uint16_t port, std::shared_ptr<SharedDirectory> shdir,
                     int timeout);

  void on_list(Poll &poll, dto::Simple &simple);
  void on_download(Poll &poll, dto::Simple &simple);
  void on_upload(Poll &poll, dto::Complex &complex);
  void on_delete(Poll &poll, dto::Simple &simple);
  template<typename T>
  void respond(Poll &p, T dto);
  void on_output(Poll &p) override;
};

#endif //DISTFS_UDPSERVER_H
