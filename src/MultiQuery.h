//
// Created by Samvel Abrahamyan on 2019-05-30.
//

#ifndef DISTFS_MULTIQUERY_H
#define DISTFS_MULTIQUERY_H

#include "nio/Subscriber.h"
#include <functional>

template<class REQ, class RES>
class MultiQuery : public Subscriber {
	REQ req;
	std::function<void(RES &, sockaddr_in)> callback;
	std::function<void(void)> error;
	std::function<void(void)> done;
	std::string addr;
	int timeout;
	struct sockaddr_in remote;
public:
	MultiQuery(uint16_t port, std::string addr, int timeout);

	void execute(REQ &req_str, std::function<void(RES &, sockaddr_in)> callback,
	             std::function<void(void)> error,
	             std::function<void(void)> done);

	void on_output(Poll &p) override;

	void on_input(Poll &p) override;

	void on_error(Poll &p, int event) override;

};


#endif //DISTFS_MULTIQUERY_H
