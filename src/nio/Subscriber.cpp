#include <utility>
#include "Subscriber.h"


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

Subscriber::Subscriber(std::string name) : name(std::move(name)) {}

const std::string &Subscriber::get_name() const {
	return name;
}

void Subscriber::on_error(Poll &p, int event) {
	std::cout << this->name << ": on_error event_code=" << event << " errno=" << from_errno() << std::endl;
	p.unsubscribe(*this);
}

void Subscriber::on_input(Poll &p) {

}

void Subscriber::on_output(Poll &p) {

}
