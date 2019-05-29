//
// Created by Samvel Abrahamyan on 2019-05-26.
//

#ifndef DISTFS_POLL_H
#define DISTFS_POLL_H

#include "Subscriber.h"
#include "Error.h"

class Subscriber;

class Poll {
private:
	static constexpr int WAIT_ENDLESS = -1;
	std::vector<pollfd> fds;
	std::unordered_map<int, std::shared_ptr<Subscriber>> subs;
	bool shutdown;


public:

	void subscribe(std::shared_ptr<Subscriber> sub);

	void unsubscribe(Subscriber &sub);

	void do_poll();

	void do_shutdown();

	Poll();
};


#endif //DISTFS_POLL_H
