//
// Created by Samvel Abrahamyan on 2019-05-26.
//

#include "Poll.h"

const int YES = 1;

void Poll::subscribe(std::shared_ptr<Subscriber> sub) {
	if (sub->get_fd() == 0 || sub->get_mask() == 0) {
		throw Error("Zero mask or fd");
	}
	no_err(ioctl(sub->get_fd(), FIONBIO, (char *) &YES), "Setting to non blocking");
	fds.push_back({.fd = sub->get_fd(), .events = sub->get_mask(), .revents = 0});
	subs[sub->get_fd()] = sub;
}

void Poll::unsubscribe(Subscriber &sub) {
	int fd = sub.get_fd();
	auto it = subs.find(fd);
	if (it == this->subs.end()) {
		throw Error("Subscriber already unsubscribed");
	}
	this->subs.erase(it);
	for (auto &i: this->fds) {
		if (i.fd == fd) {
			i.fd *= -1;
		}
	}
}

void Poll::do_poll() {
	while (!this->shutdown) {
		int changed = no_err(poll(&fds[0], fds.size(), WAIT_ENDLESS), "error in poll");
		for (auto fd: this->fds) {
			if (fd.revents == 0) continue;
			auto listener = this->subs.find(fd.fd);
			if (listener == this->subs.end()) {
				throw Error("No handler for fd %d", fd.fd);
			}
			switch (fd.revents) {
				case (POLLIN): {
					listener->second->on_input(*this);
					break;
				}
				case (POLLOUT): {
					listener->second->on_output(*this);
					break;
				}
				default: {
					listener->second->on_error(*this, fd.revents);
					break;
				}
			}
		}
	}
}

Poll::Poll() : shutdown(false) {}

void Poll::do_shutdown() {
	this->shutdown = true;
}