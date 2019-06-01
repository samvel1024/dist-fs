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
      std::cout << "Could not write to file " << file << std::endl;
      p.unsubscribe(*this);
    }
  } else {
    stream.close();
    if (this->success)
      success(file);
    p.unsubscribe(*this);
  }
}
TCPServer::SessionFactory FileReceiveSession::create_session_factory(
    boost::filesystem::path target,
    OnSuccess success) {
  return [=](Poll &p, TCPServer &server, sockaddr_in ad) -> std::shared_ptr<Subscriber> {
    p.unsubscribe(server);
    return std::make_shared<FileReceiveSession>(target, success);
  };
}
void FileReceiveSession::on_error(Poll &p, int e) {
  Subscriber::on_error(p, e);
  //TODO add listener and notify
}
