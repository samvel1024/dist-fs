//
// Created by Samvel Abrahamyan on 2019-05-26.
//

#ifndef DISTFS_TCPSERVER_H
#define DISTFS_TCPSERVER_H

#include "Subscriber.h"


class TCPSession : public Subscriber {

private:
	static constexpr int BUFLEN = 2048;
	char buffer[BUFLEN];
	int buf_sent;
	int buf_size;

public:

	void on_input(Poll &p) override;

	void on_output(Poll &p) override;

	~TCPSession() override = default;

	explicit TCPSession(std::string nm);
};

class TCPServer : public Subscriber {

public:

	void on_input(Poll &p) override;

	void on_output(Poll &p) override;

	~TCPServer() override = default;

	explicit TCPServer(std::string nm, int port = 0);
};


#endif //DISTFS_TCPSERVER_H
