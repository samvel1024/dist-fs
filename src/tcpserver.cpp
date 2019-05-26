#include "nio/Poll.h"
#include "nio/TCPServer.h"



int main() {
	Poll poll;
	std::shared_ptr<TCPServer> server = std::make_shared<TCPServer>(6000);
	poll.subscribe(server);
	try {
		poll.do_poll();
	} catch (std::exception &e) {
		std::cout << e.what() << std::endl;
	}
}
