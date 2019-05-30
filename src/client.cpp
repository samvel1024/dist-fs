
#include "nio/Poll.h"
#include "nio/Subscriber.h"
#include "CLIListener.h"
#include "nio/KillReceiver.h"


int main() {
	Poll p;
	p.subscribe(std::make_shared<CLIListener>())
		.subscribe(std::make_shared<KillReceiver>());
	p.loop();
}

