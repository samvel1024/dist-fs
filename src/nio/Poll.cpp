#include "Poll.h"
#include <signal.h>
#include <string.h>

const int YES = 1;

Poll &Poll::subscribe(std::shared_ptr<Subscriber> sub) {
	if (sub->get_fd() >= 0)
		no_err(ioctl(sub->get_fd(), FIONBIO, (char *) &YES), "Setting to non blocking");
	fds.push_back({.fd = sub->get_fd(), .events = sub->get_mask(), .revents = 0});
	subs[sub->get_fd()] = sub;
	sub->bind_pollfd(&fds[fds.size() - 1]);
	return *this;
}

void Poll::unsubscribe(Subscriber &sub) {
	int fd = sub.get_fd();
	auto it = subs.find(fd);
	if (it == this->subs.end()) {
		throw Error("Subscriber already unsubscribed");
	}
	this->subs.erase(it);
	for (auto &i: this->fds) { //TODO remove not needed fds
		if (i.fd == fd) {
			i.fd *= -1;
			if (i.fd == 0) i.fd = -1;
		}
	}
}

void Poll::loop() {
	while (!this->shutdown) {
		int changed_fds = no_err(poll(&fds[0], fds.size(), WAIT_QUANTUM), "error in poll");
		if (changed_fds == 0) { //Timeout occured
			uint64_t now = current_time_millis();
			for (auto it = alarms.cbegin(); it != alarms.cend() && it->first <= now;) {
				it->second->get_callback()();
				alarms.erase(it++);
			}
			continue;
		}
		for (int i = 0; i < fds.size(); ++i) {
			auto &fd = fds[i];
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

Poll::Poll() : shutdown(false) {
	struct sigaction sa;
	bzero(&sa, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	no_err(sigaction(SIGPIPE, &sa, 0), "Could not ignore sigpipe");
}

void Poll::do_shutdown() {
	this->shutdown = true;
}

Poll &Poll::subscribe_alarm(std::shared_ptr<Alarm> alarm) {
	this->alarms[alarm->get_timeout_time()] = alarm;
	return *this;
}
