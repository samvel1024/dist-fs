//
// Created by Samvel Abrahamyan on 2019-05-30.
//

#include "CLIListener.h"

CLIListener::CLIListener() : Subscriber("CLIListener") {
	set_fd(STDIN_FILENO);
	set_expected(POLLIN);
}


void CLIListener::on_input(Poll &p) {
	std::cout << "New input" << std::endl;
	char buf[10000];
	int r = read(fd, buf, 10000);
	buf[r] = '\0';
	std::cout << "read " << r << " " << buf << std::endl;
	set_expected(0);
	p.subscribe_alarm(std::make_shared<Alarm>(
		2000,
		[&]() {
			set_expected(POLLIN);
		}
	));
}


