//
// Created by Samvel Abrahamyan on 2019-05-27.
//

#ifndef DISTFS_FILEDOWNLOADSESSION_H
#define DISTFS_FILEDOWNLOADSESSION_H

#include "nio/TCPServer.h"
#include "nio/SendBuffer.h"
#include <boost/filesystem.hpp>
#include <functional>

class FileSendSession : public Subscriber {
 public:
 typedef std::function<void(boost::filesystem::path)> OnSuccess;
 typedef std::function<void(boost::filesystem::path)> OnError;

 private:
  SendBuffer buff;
  boost::filesystem::fstream stream;
  OnSuccess success{};
  OnError error{};
  boost::filesystem::path file;
 public:

  void on_input(Poll &p) override;

  void on_output(Poll &p) override;

  ~FileSendSession() override;

  explicit FileSendSession(const boost::filesystem::path &file);

  static TCPServer::SessionFactory create_session_factory(const boost::filesystem::path &p);

  void when_success(OnSuccess s);

  void when_error(OnError er);
  void on_error(Poll &p, int event) override;
};

#endif //DISTFS_FILEDOWNLOADSESSION_H
