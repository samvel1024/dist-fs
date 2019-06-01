
#include "nio/Poll.h"
#include "nio/TCPServer.h"
#include "FileSendSession.h"
#include "FileReceiveSession.h"
#include <iostream>

namespace fs = boost::filesystem;

class Stdin : public Subscriber {
 public:

  Stdin() : Subscriber("qa") {}
  void on_input(Poll &p) override {
    char ch;
    std::string line;
    int sz;
    while ((sz = read(fd, &ch, 1)) == 1 && ch != '\n') {
      line += ch;
    }
    if (sz == 0) {
      std::cout << "end" << std::endl;
      p.unsubscribe(*this);
    }
    std::cout << line << std::endl;
  }
};

int main(int ac, char **av) {
  Poll p;
  auto s = std::make_shared<Stdin>();
  s->set_fd(STDIN_FILENO);
  s->set_expected(POLLIN);
  p.subscribe(s);
  p.loop();
}