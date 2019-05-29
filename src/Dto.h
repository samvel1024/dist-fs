#ifndef DISTFS_DTO_H
#define DISTFS_DTO_H

#include <cstdint>
#include <string>
#include <memory>
#include <arpa/inet.h>
#include <unordered_map>
#include <string>
#include <iostream>
#include <string.h>
#include <sstream>

#ifdef __linux__
#if __BIG_ENDIAN__
# define htonll(x) (x)
# define ntohll(x) (x)
#else
# define htonll(x) (((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
# define ntohll(x) (((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif
#endif



namespace dto {
	constexpr int CMD_TYPE_LEN = 10;

	struct Simple {
		char cmd[CMD_TYPE_LEN];
		uint64_t cmd_seq;
		char payload[0];
	}__attribute__((packed));


	struct Complex {
		char cmd[CMD_TYPE_LEN];
		uint64_t cmd_seq;
		uint64_t param;
		char payload[0];
	}__attribute__((packed));

	enum Type {
		TYPE_UNKNOWN = 0,
		HELLO_REQ = 1, HELLO_RES,
		LIST_REQ, LIST_RES,
		DOWNLOAD_REQ, DOWNLOAD_RES,
		DEL_REQ, DEL_RES,
		UPLOAD_REQ, UPLOAD_RES
	};

	inline void do_ntoh(Complex *msg) {
		msg->cmd_seq = ntohll(msg->cmd_seq);
		msg->param = ntohll(msg->param);
	}

	inline void do_hton(Complex *msg) {
		msg->cmd_seq = htonll(msg->cmd_seq);
		msg->param = htonll(msg->param);
	}

	inline void do_ntoh(Simple *msg) {
		msg->cmd_seq = ntohll(msg->cmd_seq);
	}

	inline void do_hton(Simple *msg) {
		msg->cmd_seq = htonll(msg->cmd_seq);
	}


	inline Type from_header(std::string &header) {
		static std::unordered_map<std::string, Type> map{
			{"HELLO",    HELLO_REQ}, //TODO fill the rest
			{"GOOD_DAY", HELLO_RES}
		};
		auto it = map.find(header);
		if (it == map.end()) {
			return TYPE_UNKNOWN;
		}
		return it->second;
	}

	template<typename T>
	inline std::shared_ptr<T> create_dto(int payload_size) {
		int packet_size = sizeof(T) + payload_size;
		char *mem = (char *) malloc(packet_size);
		bzero(mem, CMD_TYPE_LEN);
		std::shared_ptr<T> ans(reinterpret_cast<T *>(mem), free);
		return ans;
	}


	template<typename T>
	inline std::string marshall(T &simple, int payload_len) {
		do_hton(&simple);
		std::string str(sizeof(T) + payload_len, '\0');
		memcpy(&str[0], &simple, sizeof(T) + payload_len);
		return str;
	}

	template<typename T>
	inline std::shared_ptr<T> unmarshall(std::string &payload) {
		char *mem = (char *) malloc(payload.size() + 1);
		mem[payload.size()] = '\0';
		std::shared_ptr<T> sp(reinterpret_cast<T *>(mem), free);
		memcpy(sp.get(), &payload[0], payload.size());
		do_ntoh(sp.get());
		return sp;
	}

}


inline std::ostream &operator<<(std::ostream &os, const dto::Simple &m) {
	return os << "dto::Simple{cmd='" << m.cmd << "', cmd_seq=" << m.cmd_seq << ", payload='" << m.payload << "'}";
}

inline std::ostream &operator<<(std::ostream &os, const dto::Complex &m) {
	return os << "dto::Complex{cmd='" << m.cmd << "', cmd_seq=" << m.cmd_seq << ", param=" << m.param << ", payload='"
	          << m.payload << "'}";
}


#endif //DISTFS_DTO_H
