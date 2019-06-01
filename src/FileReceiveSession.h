#ifndef DISTFS_SRC_FILERECEIVESESSION_H_
#define DISTFS_SRC_FILERECEIVESESSION_H_

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include "nio/Subscriber.h"
#include "nio/TCPServer.h"

class FileReceiveSession : public Subscriber {
 public:
  typedef std::function<void(boost::filesystem::path)> OnSuccess;
 private:
  boost::filesystem::fstream stream;
  boost::filesystem::path file;
  OnSuccess success;
 public:
  explicit FileReceiveSession(const boost::filesystem::path &file, OnSuccess f);

  virtual ~FileReceiveSession();

  void on_input(Poll &p) override;

  void on_error(Poll &p, int e) override;

  static TCPServer::SessionFactory create_session_factory(boost::filesystem::path target, OnSuccess success);
};

#endif //DISTFS_SRC_FILERECEIVESESSION_H_
