#include <utility>

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
#include "nio/Error.h"

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

	struct SimpleHeader {
		char cmd[CMD_TYPE_LEN];
		uint64_t cmd_seq;
	}__attribute__((packed));


	struct ComplexHeader {
		char cmd[CMD_TYPE_LEN];
		uint64_t cmd_seq;
		uint64_t param;
	}__attribute__((packed));

	struct Simple {
		SimpleHeader header;
		std::string payload;
	};

	struct Complex {
		ComplexHeader header;
		std::string payload;
	};

	enum Type {
		TYPE_UNKNOWN = 0,
		HELLO_REQ = 1, HELLO_RES,
		LIST_REQ, LIST_RES,
		DOWNLOAD_REQ, DOWNLOAD_RES,
		DEL_REQ, DEL_RES,
		UPLOAD_REQ, UPLOAD_RES,
	};

	inline void do_ntoh(ComplexHeader *msg) {
		msg->cmd_seq = ntohll(msg->cmd_seq);
		msg->param = ntohll(msg->param);
	}

	inline void do_hton(ComplexHeader *msg) {
		msg->cmd_seq = htonll(msg->cmd_seq);
		msg->param = htonll(msg->param);
	}

	inline void do_ntoh(SimpleHeader *msg) {
		msg->cmd_seq = ntohll(msg->cmd_seq);
	}

	inline void do_hton(SimpleHeader *msg) {
		msg->cmd_seq = htonll(msg->cmd_seq);
	}


	inline Type from_header(std::string &header) {
		static std::unordered_map<std::string, Type> map{
			{"HELLO",    HELLO_REQ}, //TODO fill the rest
			{"GOOD_DAY", HELLO_RES},
			{"LIST",     LIST_REQ},
			{"MY_LIST",  LIST_RES}
		};
		std::string norm = std::string(header.c_str());
		auto it = map.find(norm);
		if (it == map.end()) {
			return TYPE_UNKNOWN;
		}
		return it->second;
	}

	template<typename T>
	inline T create_dto_common(uint64_t cmd_seq, std::string cmd, std::string &payload) {
		T dto;
		bzero(dto.header.cmd, CMD_TYPE_LEN);
		dto.payload = std::move(payload);
		strcpy(dto.header.cmd, &cmd[0]);
		dto.header.cmd_seq = cmd_seq;
		return dto;
	}

	inline Complex create(uint64_t cmd_seq, std::string cmd, std::string &payload, uint64_t param) {
		auto dto = create_dto_common<Complex>(cmd_seq, std::move(cmd), payload);
		dto.header.param = param;
		return dto;
	}

	inline Simple create(uint64_t cmd_seq, std::string cmd, std::string &payload) {
		return create_dto_common<Simple>(cmd_seq, cmd, payload);
	}

	template<typename T>
	inline std::string marshall(T &dto) {
		do_hton(&(dto.header));
		int hdr_size = sizeof(decltype(dto.header));
		std::string str(hdr_size + dto.payload.size(), '\0');
		memcpy(&str[0], &(dto.header), hdr_size);
		memcpy(&str[hdr_size], &dto.payload[0], dto.payload.size());
		do_ntoh(&(dto.header));
		return str;
	}

	template<typename T>
	inline T unmarshall(std::string &packet) {
		T dto;
		int hdr_size = sizeof(decltype(dto.header));
		if (packet.size() < hdr_size) {
			throw Error("Too small packet");
		}
		memcpy(&dto.header, &packet[0], hdr_size);
		dto.payload = std::string(packet.size() - hdr_size, '\0');
		memcpy(&dto.payload[0], &packet[hdr_size], packet.size() - hdr_size);
		do_ntoh(&dto.header);
		return dto;
	}

	template<typename T>
	inline bool equals(T &l, T &r) {
		int mcp = memcmp(&l.header, &r.header, sizeof(decltype(l.header)));
		return mcp == 0 && l.payload == r.payload;
	}

}


inline std::ostream &operator<<(std::ostream &os, const dto::Simple &m) {
	return os << "dto::Simple{cmd='" << m.header.cmd << "', cmd_seq=" << m.header.cmd_seq << ", payload='" << m.payload
	          << "'}";
}

inline std::ostream &operator<<(std::ostream &os, const dto::Complex &m) {
	return os << "dto::Complex{cmd='" << m.header.cmd << "', cmd_seq=" << m.header.cmd_seq << ", param=" << m.header.param
	          << ", payload='"
	          << m.payload << "'}";
}

inline bool operator==(const dto::Simple &lhs, const dto::Simple &rhs) {
	return dto::equals(lhs, rhs);
}

inline bool operator==(const dto::Complex &lhs, const dto::Complex &rhs) {
	return dto::equals(lhs, rhs);
}


#endif //DISTFS_DTO_H
