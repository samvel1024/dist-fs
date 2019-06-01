//
// Created by Samvel Abrahamyan on 2019-05-30.
//

#ifndef DISTFS_MULTIQUERY_H
#define DISTFS_MULTIQUERY_H

#include "nio/Subscriber.h"
#include <functional>

template<class REQ, class RES>
class MultiQuery : public Subscriber {
 public:
  typedef std::function<void(RES &, sockaddr_in)> OnResponse;
  typedef std::function<void(void)> OnError;
  typedef std::function<void(void)> OnTimeout;
 private:
  REQ req;
  OnResponse callback{};
  OnError error{};
  OnTimeout done{};
  std::string addr;
  int timeout;
  struct sockaddr_in remote;
 public:

  MultiQuery(REQ &req, uint16_t port, std::string addr, int timeout);

  void on_output(Poll &p) override;

  void on_input(Poll &p) override;

  void on_error(Poll &p, int event) override;

  void when_response(OnResponse resp);

  void when_error(OnError err);

  void when_timeout(OnTimeout t);

};

#endif //DISTFS_MULTIQUERY_H
