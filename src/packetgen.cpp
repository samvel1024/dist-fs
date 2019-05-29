
#include "Dto.h"
#include <iostream>
#include <string>


using namespace dto;

int main() {
	int is_simple;
	uint64_t param, seq;
	std::string payload, cmd;
	std::getline(std::cin, cmd);
	std::getline(std::cin, payload);
	std::cin >> is_simple >> param >> seq;

	std::string serial;
	if(is_simple){
		auto dto = create_dto<Simple>(payload.size());
		strcpy(dto->cmd, &cmd[0]);
		dto->cmd_seq = seq;
		strcpy(dto->payload, &payload[0]);
		std::cerr << *dto << std::endl;
		serial = marshall(*dto, payload.size());
	}else {
		auto dto = create_dto<Complex>(payload.size());
		strcpy(dto->cmd, &cmd[0]);
		dto->cmd_seq = seq;
		dto->param= param;
		strcpy(dto->payload, &payload[0]);
		std::cerr << *dto << std::endl;
		serial = marshall(*dto, payload.size());
	}

	std::cout << serial;

	return 0;
}