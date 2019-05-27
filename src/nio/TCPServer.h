//
// Created by Samvel Abrahamyan on 2019-05-26.
//

#ifndef DISTFS_TCPSERVER_H
#define DISTFS_TCPSERVER_H

#include "Subscriber.h"
#include "TCPSessionFactory.h"

class TCPSessionFactory;

class TCPServer : public Subscriber {

	std::shared_ptr<TCPSessionFactory> fctr;
public:

	void on_input(Poll &p) override;

	void on_output(Poll &p) override;

	~TCPServer() override = default;

	explicit TCPServer(std::string nm, std::shared_ptr<TCPSessionFactory> ft, int port = 0);
};



#endif //DISTFS_TCPSERVER_H
