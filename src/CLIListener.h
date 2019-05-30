#ifndef DISTFS_CLILISTENER_H
#define DISTFS_CLILISTENER_H

#include "nio/Subscriber.h"
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

class CLIListener : public Subscriber {
private:
	uint16_t port;
	fs::path out_dir;
	int timeout_sec;
	std::string mcast_addr;
public:
	CLIListener(uint16_t port, boost::filesystem::path out_dir, int timeout_sec,
	            std::string mcast_addr);

	void on_input(Poll &p) override;

	void on_error(Poll &p, int event) override;
};


#endif //DISTFS_CLILISTENER_H
