#ifndef DISTFS_CLILISTENER_H
#define DISTFS_CLILISTENER_H

#include "nio/Subscriber.h"

class CLIListener : public Subscriber{

public:
	CLIListener();

	void on_input(Poll &p) override;
};


#endif //DISTFS_CLILISTENER_H
