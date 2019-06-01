#include "FileReceiveSession.h"

namespace fs = boost::filesystem;

FileReceiveSession::FileReceiveSession(const fs::path &file, OnSuccess f) :
    Subscriber("FileReceiveSession"), stream(), file(file), success(std::move(f)) {
    set_expected(POLLIN);
    stream.open(file, std::ios::out);
}

FileReceiveSession::~FileReceiveSession() {
  if (stream.is_open())
    stream.close();
}

void FileReceiveSession::on_input(Poll &p) {
  char buff[4000];
  int bytes = read(fd, buff, 4000);
  if (bytes < 0) {
    //TODO print format;
    std::cout << "Error in reading from socket " << std::endl;
    p.unsubscribe(*this);
    return;
  } else if (bytes > 0) {
    stream.write(buff, bytes);
    if (!stream) {
      //TODO error msg
      std::cout << "Could not write to file " << file <<  std::endl;
      p.unsubscribe(*this);
    }
  } else {
    stream.close();
    success(file);
    p.unsubscribe(*this);
  }
}