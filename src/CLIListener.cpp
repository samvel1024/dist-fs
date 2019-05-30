#include <utility>

#include <utility>

//
// Created by Samvel Abrahamyan on 2019-05-30.
//

#include "CLIListener.h"


void CLIListener::on_input(Poll &p) {
	std::cout << "New input" << std::endl;
	char buf[10000];
	int r = read(fd, buf, 10000);
	buf[r] = '\0';
	std::cout << "read " << r << " " << buf << std::endl;
	set_expected(0);
	p.notify_subscriber_changed(*this);
	p.subscribe_alarm(std::make_shared<Alarm>(
		2000,
		[&]() {
			set_expected(POLLIN);
			p.notify_subscriber_changed(*this);
		}
	));
}

void CLIListener::on_error(Poll &p, int event) {
	if (event & POLLHUP) {
		std::cout << name << ": Ignoring POLLHUP" << std::endl;
		return;
	}
	Subscriber::on_error(p, event);
}

CLIListener::CLIListener(uint16_t port, fs::path out_dir, int timeout_sec, std::string mcast_addr) :
	Subscriber("CLIListener"), port(port),
	out_dir(std::move(out_dir)), timeout_sec(timeout_sec),
	mcast_addr(std::move(mcast_addr)) {
	set_fd(STDIN_FILENO);
	set_expected(POLLIN);
}


