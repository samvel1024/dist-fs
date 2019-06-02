#include "Poll.h"
#include <signal.h>
#include <string.h>
#include <cmath>

const int YES = 1;

Poll &Poll::subscribe(std::shared_ptr<Subscriber> sub) {
  if (sub->get_fd() >= 0)
    no_err(ioctl(sub->get_fd(), FIONBIO, (char *) &YES), "Setting to non blocking");
  fds.push_back({.fd = sub->get_fd(), .events = sub->get_mask(), .revents = 0});
  subs[sub->get_fd()] = sub;
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
  uint64_t time = current_time_millis();
  while (!this->shutdown && this->subs.size() > 0) {
    int changed_fds = no_err(poll(&fds[0], fds.size(), WAIT_QUANTUM), "error in poll");
    if (changed_fds == 0 || current_time_millis() - time > WAIT_QUANTUM) { //Timeout occured
      time = current_time_millis();
      for (auto it = alarms.cbegin(); it != alarms.cend() && it->first <= time;) {
        it->second->on_timeout();
        alarms.erase(it++);
      }
      continue;
    }
    for (int i = 0; i < fds.size(); ++i) {
      auto fd = fds[i];
      if (fd.revents == 0) continue;
      auto listener_itr = this->subs.find(fd.fd);
      if (listener_itr == this->subs.end()) {
        throw Error("No handler for fd %d", fd.fd);
      }
      std::shared_ptr<Subscriber> listener = listener_itr->second;
      try {
        if (fd.revents & POLLIN) {
          listener->on_input(*this);
        }
        if (fd.revents & POLLOUT) {
          listener->on_output(*this);
        }
        if (fd.revents & !(POLLIN | POLLOUT)) {
          std::cout << (fd.revents ^ (POLLIN | POLLOUT)) << " " << fd.revents << std::endl;
          listener->on_error(*this, fd.revents);
        }
      }catch (Error &e){
        printf("Error in event loop %s listener=%s pollfd{ev=%d, fd=%d, revents=%d}\n",
        e.what(), &listener->get_name()[0], fd.events, fd.fd, fd.revents);
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

void Poll::notify_subscriber_changed(Subscriber &listener) {
  if (!listener.is_dirty()) return;
  int initial_fd = abs(listener.get_fd());
  for (auto &fd: fds) {
    if (fd.fd == initial_fd) {
      fd.fd = listener.get_fd();
      fd.events = listener.get_mask();
      listener.set_dirty(false);
      break;
    }
  }
}
