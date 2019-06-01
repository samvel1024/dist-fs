//
// Created by Samvel Abrahamyan on 2019-05-27.
//

#ifndef DISTFS_FILEDOWNLOADSESSION_H
#define DISTFS_FILEDOWNLOADSESSION_H

#include "nio/TCPServer.h"
#include "nio/TCPSessionFactory.h"
#include "nio/SendBuffer.h"
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

class FileDownloadSession : public Subscriber {

 private:
  int timeout;
  SendBuffer buff;
  fs::fstream stream;
 public:

  void on_input(Poll &p) override;

  void on_output(Poll &p) override;

  ~FileDownloadSession() override;

  explicit FileDownloadSession(const fs::path &file, int timeout);

  static TCPServer::SessionFactory create_session_factory(const fs::path &p, int timeout);

};

#endif //DISTFS_FILEDOWNLOADSESSION_H
