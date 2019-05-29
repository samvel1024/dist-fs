//
// Created by Samvel Abrahamyan on 2019-05-27.
//

#ifndef DISTFS_UDPSERVER_H
#define DISTFS_UDPSERVER_H


#include "nio/Subscriber.h"
#include "Dto.h"

class UDPServer : public Subscriber {
private:
	static constexpr int buf_len = 2000;
	std::string buffer;

	void on_dispatch(Poll &p, int bytes_read);
	void on_hello(Poll &p, dto::Simple &msg);
public:

	void on_input(Poll &p) override;

	void on_output(Poll &p) override;

	~UDPServer() override;

	explicit UDPServer(const std::string name, std::string addr, uint16_t port);

};


#endif //DISTFS_UDPSERVER_H
