//
// Created by Samvel Abrahamyan on 2019-05-30.
//

#ifndef DISTFS_SENDBUFFER_H
#define DISTFS_SENDBUFFER_H

#include <string>
#include "Error.h"



class SendBuffer {

	std::string buf;
	int full;
	int read;

public:

	SendBuffer(int size) {
		this->buf = std::string(size, '\0');
		full = 0;
		read = 0;
	}

	char *read_pos(){
		if (full == 0){
			throw Error("Buffer is empty");
		}
		return &buf[read];
	}

	void on_read_bytes(int bt){
		read += bt;
		if(read > full){
			throw Error("Read more bytes than was full");
		}
	}

	int remaining_to_read(){
		return full - read;
	}

	bool is_all_read(){
		return read > 0 && read == full;
	}

	void load_bytes(char *source, int bytes){
		if (bytes > buf.size())
			throw Error("Too many bytes to read");
		for(int p = 0; p < bytes; ++p){
			buf[p] = *(source + p);
		}
		full = bytes;
		read = 0;
	}

};


#endif //DISTFS_SENDBUFFER_H
