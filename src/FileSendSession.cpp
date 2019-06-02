#include <utility>

#include <utility>

#include <utility>

//
// Created by Samvel Abrahamyan on 2019-05-27.
//

#include "FileSendSession.h"

const int FILE_SEND_BUF_SIZE = 10000;
char FILE_SEND_BUF[FILE_SEND_BUF_SIZE];

namespace fs = boost::filesystem;

void FileSendSession::on_input(Poll &p) {

}

void FileSendSession::on_output(Poll &p) {
  if (buff.is_all_read()) {
    stream.read(FILE_SEND_BUF, FILE_SEND_BUF_SIZE);
    buff.load_bytes(FILE_SEND_BUF, stream.gcount());
    if (!stream && !stream.eof()) {
      std::cout << this->name << ": Error in reading from file, disconnecting\n";
      if (this->error)
        this->error(file);
      p.unsubscribe(*this);
      return;
    }
  }
  int n = write(this->get_fd(), buff.read_pos(), buff.remaining_to_read());
  if (n < 0) {
    std::cout << this->name << ": Error in writing to socket, disconnecting\n";
    if (this->error)
      this->error(file);
    p.unsubscribe(*this);
    return;
  }
  buff.on_read_bytes(n);
  if (buff.is_all_read() && stream.eof()) {
    if (this->success)
      this->success(file);
    p.unsubscribe(*this);
  }
}

FileSendSession::FileSendSession(const fs::path &file) :
    Subscriber("FileDownload"),
    buff(10000), stream(file), file(file) {
  set_expected(POLLOUT);
}

TCPServer::SessionFactory FileSendSession::create_session_factory(const fs::path &path) {
  return [=](Poll &p, TCPServer &server, sockaddr_in client) -> std::shared_ptr<Subscriber> {
    p.unsubscribe(server);
    auto s = std::make_shared<FileSendSession>(path);
    s->when_success([](auto a) { std::cout << "Sent file " << a << std::endl; });
    s->when_error([](auto a) { std::cout << "Could not send file " << a << std::endl; });
    return s;
  };
}

FileSendSession::~FileSendSession() {
  if (stream.is_open())
    stream.close();
}

void FileSendSession::when_success(FileSendSession::OnSuccess s) {
  this->success = std::move(s);
}

void FileSendSession::when_error(FileSendSession::OnError er) {
  this->error = std::move(er);
}

void FileSendSession::on_error(Poll &p, int event) {
  if (this->error)
    this->error(file);
  Subscriber::on_error(p, event);
}

