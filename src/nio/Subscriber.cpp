//
// Created by Samvel Abrahamyan on 2019-05-26.
//

#include "Subscriber.h"


void Subscriber::on_error(Poll &p, int event) {
	std::cout << this->name << ": on_error event_code=" << event << " errno=" << from_errno() << std::endl;
	p.unsubscribe(*this);
}


Subscriber::~Subscriber() {
	if (this->fd != 0) {
		std::cout << "Closing fd for " << this->name << std::endl;
		close(this->fd);
	}
}

int Subscriber::get_fd() const {
	return fd;
}

void Subscriber::set_fd(int mdf) {
	this->fd = mdf;
}

short Subscriber::get_mask() const {
	return expected;
}

void Subscriber::set_expected(short mmask) {
	this->expected = mmask;
}

Subscriber::Subscriber(const std::string &name) : name(name) {}

