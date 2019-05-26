//
// Created by Samvel Abrahamyan on 2019-05-26.
//

#ifndef DISTFS_KILLRECEIVER_H
#define DISTFS_KILLRECEIVER_H


#include "Subscriber.h"

class KillReceiver : public Subscriber {
public:

	void on_input(Poll &p) override;

	void on_output(Poll &p) override;

	KillReceiver();
};


#endif //DISTFS_KILLRECEIVER_H
