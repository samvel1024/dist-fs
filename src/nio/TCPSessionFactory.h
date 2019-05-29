//
// Created by Samvel Abrahamyan on 2019-05-27.
//

#ifndef DISTFS_TCPSESSIONFACTORY_H
#define DISTFS_TCPSESSIONFACTORY_H

#include "Subscriber.h"
#include "TCPServer.h"

class TCPServer;

class TCPSessionFactory {
public:
	virtual std::shared_ptr<Subscriber> create_session(TCPServer &serv, int fd) = 0;
	TCPSessionFactory() = default;
	virtual ~TCPSessionFactory() = default;
};


#endif //DISTFS_TCPSESSIONFACTORY_H
