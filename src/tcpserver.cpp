#include "nio/Poll.h"
#include "nio/TCPServer.h"
#include "nio/KillReceiver.h"


int main() {
	Poll poll;
	std::shared_ptr<TCPServer> server = std::make_shared<TCPServer>("TCP_6000", 6000);
	poll.subscribe(server);

	std::shared_ptr<TCPServer> server2 = std::make_shared<TCPServer>("TCP_6001", 6001);
	poll.subscribe(server2);

#ifdef __linux__
	std::shared_ptr<KillReceiver> rec = std::make_shared<KillReceiver>();
	poll.subscribe(rec);
#endif
	try {
		poll.do_poll();
	} catch (std::exception &e) {
		std::cout << e.what() << std::endl;
	}
}
