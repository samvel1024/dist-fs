#ifndef DISTFS_SRC_FILERECEIVESESSION_H_
#define DISTFS_SRC_FILERECEIVESESSION_H_

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include "nio/Subscriber.h"

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

 private:
  void on_input(Poll &p) override;
};

#endif //DISTFS_SRC_FILERECEIVESESSION_H_
