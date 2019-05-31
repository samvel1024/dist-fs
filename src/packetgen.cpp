
#include "Dto.h"
#include <iostream>
#include <string>


using namespace dto;

/*
 * example usage  printf "LIST\nDownload\n1 4123 1234" | packetgen | nc -u ${IP} 3000
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
		auto dto = create_dto<Simple>(payload.size(), 0);
		strcpy(dto->cmd, &cmd[0]);
		dto->cmd_seq = seq;
		strcpy(dto->payload, &payload[0]);
		std::cerr << "[PACKETGEN] " << *dto << std::endl;
		serial = marshall(*dto, payload.size());
	} else {
		auto dto = create_dto<Complex>(payload.size(), 0);
		strcpy(dto->cmd, &cmd[0]);
		dto->cmd_seq = seq;
		dto->param = param;
		strcpy(dto->payload, &payload[0]);
		std::cerr << "[PACKETGEN] " << *dto << std::endl;
		serial = marshall(*dto, payload.size());
	}

	std::cout << serial;

	return 0;
}
