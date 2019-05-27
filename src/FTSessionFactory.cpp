//
// Created by Samvel Abrahamyan on 2019-05-27.
//

#include "FTSessionFactory.h"
#include "FTSession.h"

std::shared_ptr<Subscriber> FTSessionFactory::create_session(TCPServer &serv, int fd) {
	return std::make_shared<FTSession>(serv.get_name() + "_client", fd);
}

FTSessionFactory::FTSessionFactory() {

}

FTSessionFactory::~FTSessionFactory() {

}
