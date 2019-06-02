#ifndef DISTFS_SRC_FILERECEIVESESSION_H_
#define DISTFS_SRC_FILERECEIVESESSION_H_

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include "nio/Subscriber.h"
#include "nio/TCPServer.h"

class FileReceiveSession : public Subscriber {
 public:
  typedef std::function<void(boost::filesystem::path)> OnSuccess;
  typedef std::function<void(boost::filesystem::path)> OnFail;
 private:
  boost::filesystem::fstream stream;
  boost::filesystem::path file;
  OnSuccess success;
  OnFail fail;
 public:
  explicit FileReceiveSession(const boost::filesystem::path &file);

  virtual ~FileReceiveSession();

  void on_input(Poll &p) override;

  void on_error(Poll &p, int e) override;

  void when_sucess(OnSuccess s);

  void when_error(OnFail f);

  static TCPServer::SessionFactory create_session_factory(boost::filesystem::path target);
};

#endif //DISTFS_SRC_FILERECEIVESESSION_H_
