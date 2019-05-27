//
// Created by Samvel Abrahamyan on 2019-05-27.
//

#ifndef DISTFS_FTSESSION_H
#define DISTFS_FTSESSION_H

#include "nio/TCPServer.h"
#include "nio/TCPSessionFactory.h"

class FTSession : public Subscriber {

private:
	static constexpr int BUFLEN = 10000;
	char buffer[BUFLEN]{};
	int buf_sent{};
	int buf_size{};

public:

	void on_input(Poll &p) override;

	void on_output(Poll &p) override;

	~FTSession() override = default;

	explicit FTSession(std::string nm, int fd);
};



#endif //DISTFS_FTSESSION_H
