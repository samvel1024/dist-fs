#include <utility>

#include "FileReceiveSession.h"

namespace fs = boost::filesystem;

const int FILE_REC_BUF_SIZE = 10000;
char FILE_REC_BUF[FILE_REC_BUF_SIZE];

FileReceiveSession::FileReceiveSession(const fs::path &file) :
    Subscriber("FileReceiveSession"), stream(), file(file) {
  set_expected(POLLIN);
  stream.open(file, std::ios::out);
}

FileReceiveSession::~FileReceiveSession() {
  if (stream.is_open())
    stream.close();
}

void FileReceiveSession::on_input(Poll &p) {
  int bytes = read(fd, FILE_REC_BUF, FILE_REC_BUF_SIZE);
  if (bytes < 0) {
    if (this->fail)
      this->fail(file);
    std::cout << "Error in reading from socket " << std::endl;
    p.unsubscribe(*this);
    return;
  } else if (bytes > 0) {
    stream.write(FILE_REC_BUF, bytes);
    if (!stream) {
      if (this->fail)
        this->fail(file);
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
TCPServer::SessionFactory FileReceiveSession::create_session_factory(boost::filesystem::path target) {
  return [=](Poll &p, TCPServer &server, sockaddr_in ad) -> std::shared_ptr<Subscriber> {
    p.unsubscribe(server);
    auto s = std::make_shared<FileReceiveSession>(target);
    s->when_error([](auto p) { std::cout << "Error in receiving file " << p << std::endl; });
    s->when_sucess([](auto p) { std::cout << "Received file " << p << std::endl; });
    return s;
  };
}
void FileReceiveSession::on_error(Poll &p, int e) {
  if (this->fail)
    this->fail(file);
  Subscriber::on_error(p, e);
}
void FileReceiveSession::when_error(FileReceiveSession::OnFail f) {
  this->fail = std::move(f);
}
void FileReceiveSession::when_sucess(FileReceiveSession::OnSuccess s) {
  this->success = std::move(s);
}
