//
// Created by Samvel Abrahamyan on 2019-05-27.
//

#ifndef DISTFS_FILEDOWNLOADSESSION_H
#define DISTFS_FILEDOWNLOADSESSION_H

#include "nio/TCPServer.h"
#include "nio/SendBuffer.h"
#include <boost/filesystem.hpp>

class FileSendSession : public Subscriber {

 private:
  SendBuffer buff;
  boost::filesystem::fstream stream;
 public:

  void on_input(Poll &p) override;

  void on_output(Poll &p) override;

  ~FileSendSession() override;

  explicit FileSendSession(const boost::filesystem::path &file);

  static TCPServer::SessionFactory create_session_factory(const boost::filesystem::path &p);

};

#endif //DISTFS_FILEDOWNLOADSESSION_H
