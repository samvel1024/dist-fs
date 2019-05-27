#include "nio/Poll.h"
#include "nio/TCPServer.h"
#include "nio/KillReceiver.h"
#include "nio/TCPSessionFactory.h"
#include "FTSessionFactory.h"
#include "FTSession.h"



int main() {

	std::shared_ptr<FTSessionFactory> fc = std::make_shared<FTSessionFactory>();

	Poll poll;
	std::shared_ptr<TCPServer> server = std::make_shared<TCPServer>("TCP_6000", fc, 6000);
	poll.subscribe(server);

	std::shared_ptr<TCPServer> server2 = std::make_shared<TCPServer>("TCP_6001", fc, 6001);
	poll.subscribe(server2);
	auto *tcp = new FTSessionFactory();
#ifdef __linux__
	std::shared_ptr<KillReceiver> rec = std::make_shared<KillReceiver>();
	poll.subscribe(rec);
#endif
	try {
		poll.do_poll();
	}
	catch (Error &e){
		std::cout << "Error: " << e.what() << std::endl;
	}catch (std::exception &e) {
		std::cout << "Unknown exception: " << e.what() << std::endl;
	}
}
