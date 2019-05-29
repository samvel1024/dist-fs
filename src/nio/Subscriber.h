//
// Created by Samvel Abrahamyan on 2019-05-26.
//

#ifndef DISTFS_SUBSCRIBER_H
#define DISTFS_SUBSCRIBER_H

#include "Poll.h"
#include "util.h"

class Poll;

class Subscriber {
protected:
	int fd{};
	short expected{};
	std::string name;
public:
	virtual void on_error(Poll &p, int event);

	virtual void on_input(Poll &p);

	virtual void on_output(Poll &p);

	virtual ~Subscriber();

	int get_fd() const;

	void set_fd(int mdf);

	short get_mask() const;

	void set_expected(short mmask);

	const std::string &get_name() const;

	Subscriber(std::string name);
};

#endif //DISTFS_SUBSCRIBER_H
