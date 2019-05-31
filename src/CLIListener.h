#ifndef DISTFS_CLILISTENER_H
#define DISTFS_CLILISTENER_H

#include "nio/Subscriber.h"
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

class CLIListener : public Subscriber {
private:
	uint64_t cmd_seq;
	uint16_t port;
	fs::path out_dir;
	int timeout_sec;
	std::string mcast_addr;
	std::unordered_map<std::string, int> commands;

	void exec_command(Poll &p, std::string &type, std::string &arg);

public:

	CLIListener(uint16_t port, boost::filesystem::path out_dir, int timeout_sec,
	            std::string mcast_addr);

	void on_input(Poll &p) override;

	void on_error(Poll &p, int event) override;

	void do_discover(Poll &p);

	void do_search(Poll &p, std::string &str);

	void unblock_input(Poll &p);
};

#endif //DISTFS_CLILISTENER_H
