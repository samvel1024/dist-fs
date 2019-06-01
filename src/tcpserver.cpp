
#include "nio/Poll.h"
#include "nio/TCPServer.h"
#include "FileSendSession.h"
#include <iostream>

namespace fs = boost::filesystem;

int main(int ac, char **av) {

  if (ac != 2) {
    std::cerr << "Illegal input";
    return 1;
  }

  fs::path path((std::string(av[1])));
  std::cout << path << std::endl;
  Poll p;
  auto tcp = std::make_shared<TCPServer>("TCP", FileSendSession::create_session_factory(path), 3001);
  p.subscribe(std::move(tcp));
  p.loop();
}