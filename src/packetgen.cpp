
#include "Dto.h"
#include <iostream>
#include <string.h>


int main(){
	auto d = dto::create_dto<dto::Simple>(sizeof("1234567890"));
	sprintf(d->cmd, "HELLO");
	d->cmd_seq = 1125391028313;
	sprintf(d->payload, "1234567890");

	std::string s = dto::marshall<dto::Simple>(*d, strlen(d->payload));
	std::cout << s << std::endl;
	auto unmarshalled = dto::unmarshall<dto::Simple>(s);
	std::cout << "Unrshalled: " << *unmarshalled << std::endl;
	return 0;
}
