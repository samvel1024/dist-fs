
#include "Dto.h"
#include <iostream>
#include <string>

using namespace dto;

/*
 * example usage  printf "LIST\nDownload\n1 4123 1234" | packetgen | nc -u ${IP} ${PORT}
 */

int main() {
  int is_simple;
  uint64_t param, seq;
  std::string payload, cmd;
  std::getline(std::cin, cmd);
  std::getline(std::cin, payload);
  std::cin >> is_simple >> param >> seq;

  std::string serial;
  if (is_simple) {
    Simple dto = create(seq, cmd, payload);
    std::cerr << "[PACKETGEN] " << dto << std::endl;
    serial = marshall(dto);
  } else {
    Complex dto = create(seq, cmd, payload, param);
    std::cerr << "[PACKETGEN] " << dto << std::endl;
    serial = marshall(dto);
  }

  std::cout << serial;

  return 0;
}
