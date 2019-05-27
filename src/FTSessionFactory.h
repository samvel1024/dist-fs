//
// Created by Samvel Abrahamyan on 2019-05-27.
//

#ifndef DISTFS_FTSESSIONFACTORY_H
#define DISTFS_FTSESSIONFACTORY_H

#include "nio/TCPSessionFactory.h"

class FTSessionFactory : public TCPSessionFactory {
public:
	std::shared_ptr<Subscriber> create_session(TCPServer &serv, int fd) override;
	explicit FTSessionFactory();
	~FTSessionFactory() override;
};



#endif //DISTFS_FTSESSIONFACTORY_H
