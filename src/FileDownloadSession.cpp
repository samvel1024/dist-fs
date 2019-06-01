#include <utility>

//
// Created by Samvel Abrahamyan on 2019-05-27.
//

#include "FileDownloadSession.h"

const int SEND_BUF_SIZE = 1000;
char temp_buff[SEND_BUF_SIZE];

void FileDownloadSession::on_input(Poll &p) {

}

void FileDownloadSession::on_output(Poll &p) {
  if (buff.is_all_read()) {
    stream.read(temp_buff, SEND_BUF_SIZE);
    buff.load_bytes(temp_buff, stream.gcount());
    if (!stream && !stream.eof()) {
      std::cout << this->name << ": Error in reading from file, disconnecting\n";
      p.unsubscribe(*this);
      return;
    }
  }
  int n = write(this->get_fd(), buff.read_pos(), buff.remaining_to_read());
  if (n < 0) {
    std::cout << this->name << ": Error in writing to socket, disconnecting\n";
    p.unsubscribe(*this);
    return;
  }
  buff.on_read_bytes(n);
  if (buff.is_all_read() && stream.eof()) {
    p.unsubscribe(*this);
  }
}

FileDownloadSession::FileDownloadSession(const fs::path &file, int timeout) :
    Subscriber("FileDownload"), timeout(timeout),
    buff(SEND_BUF_SIZE), stream(file) {
  set_expected(POLLOUT);
}

TCPServer::SessionFactory FileDownloadSession::create_session_factory(const fs::path &path, int timeout) {
  return [=](Poll &p, TCPServer &server, sockaddr_in client) -> std::shared_ptr<Subscriber> {
    p.unsubscribe(server);
    return std::make_shared<FileDownloadSession>(path, timeout);
  };
}

FileDownloadSession::~FileDownloadSession() {
  stream.close();
}


