//
// Created by Samvel Abrahamyan on 2019-05-27.
//

#include "FTSession.h"



void FTSession::on_input(Poll &p) {

}

void FTSession::on_output(Poll &p) {
	int n = write(this->get_fd(), this->buffer, 9000);
	if (n < 0){
		std::cout << this->name << ": Error in writing to socket, disconnecting\n";
		p.unsubscribe(*this);
	}else if(n != 9000){
		std::cout << "Illegal write: " <<n <<  std::endl;
	}
}

FTSession::FTSession(std::string nm, int fd) : Subscriber(nm) {
	this->set_fd(fd);
	this->set_expected(POLLOUT);
	memset(this->buffer, 'A', 9000);
}
