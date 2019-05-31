#include <utility>

//
// Created by Samvel Abrahamyan on 2019-05-27.
//

#include "UDPServer.h"
#include "Dto.h"

constexpr int BUFSIZE = 1000;

int connect_group(in_port_t port, const char *addr) {
	int sock = no_err(socket(AF_INET, SOCK_DGRAM, 0), "socket");
	u_int yes = 1;
	//TODO check if works on linux
	no_err(setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char *) &yes, sizeof(yes)), "reuse");
	//Connect to group
	struct ip_mreq ip_mreq{};
	ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	no_err(inet_aton(addr, &ip_mreq.imr_multiaddr) == 0, "inet_aton");
	no_err(setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &ip_mreq, sizeof ip_mreq), "setsockopt");
	//Bind to port
	struct sockaddr_in local_address{};
	local_address.sin_family = AF_INET;
	local_address.sin_addr.s_addr = htonl(INADDR_ANY);
	local_address.sin_port = htons(port);
	no_err(bind(sock, (struct sockaddr *) &local_address, sizeof local_address), "bind");
	no_err(fcntl(sock, F_SETFL, O_NONBLOCK), "set nonblock");

	return sock;
}


void UDPServer::on_input(Poll &p) {
	//TODO support partial reads
	unsigned clientlen; /* byte size of client's address */
	clientlen = sizeof(current_client);
	int bytes_read = no_err(recvfrom(fd, &buffer[0], buffer.size(), 0,
	                                 (struct sockaddr *) &current_client, &clientlen), "Error in recvfrom");
	on_dispatch(p, bytes_read);
//	std::cout << name << "Server received:" << buffer << std::endl;
//	std::string resp(5000, 'A');
//	no_err(sendto(fd, &resp[0], resp.size(), 0, (struct sockaddr *) &clientaddr, clientlen), "Error in sendto");
}

void UDPServer::on_dispatch(Poll &p, int bytes_read) {
	std::string type_header = buffer.substr(0, dto::CMD_TYPE_LEN);
	try {
		switch (dto::from_header(type_header)) {
			case dto::HELLO_REQ : {
				on_hello(p, *(dto::unmarshall<dto::Simple>(buffer, bytes_read)));
				break;
			}
			case dto::LIST_REQ : {
				on_list(p, *(dto::unmarshall<dto::Simple>(buffer, bytes_read)));
				break;
			}
			default : {
				throw Error("Illegal cmd type");
			}
		}
	} catch (Error &err) {
		char *s = inet_ntoa(current_client.sin_addr);
		uint16_t port = ntohs(current_client.sin_port);
		std::cout << "[PCKG ERROR] Skipping invalid package from " << s << ":" << port << ". " << err.what() << std::endl;
	}
}


void UDPServer::on_hello(Poll &p, dto::Simple &msg) {
	if (strlen(msg.payload) != 0) {
		throw Error("Hello should not have a payload");
	}
	std::cout << name << ": request " << msg << std::endl;
	std::string port_str = std::to_string(port);
	auto resp = dto::create_dto<dto::Complex>(port_str.size(), msg.cmd_seq);
	dto::init(*resp, port_str, "GOOD_DAY", dir->get_remaining_space());
	std::cout << name << ": response " << *resp << std::endl;
	auto resp_str = dto::marshall(*resp, port_str.size());
	no_err(sendto(fd, &resp_str[0], resp_str.size(), 0, (struct sockaddr *) &current_client,
	              sockaddr_len), "Error in sendto");
}

void UDPServer::on_list(Poll &poll, dto::Simple &msg) {
	std::string query(msg.payload);
	std::string resp = dir->search_file(query);
	std::cout << name << ": request " << msg << std::endl;
	if (resp.empty()) {
		return;
	}
	auto dto = dto::create_dto<dto::Simple>(resp.size(), msg.cmd_seq);
	dto::init(*dto, resp, "MY_LIST");
	std::cout << name << ": response " << *dto << std::endl;
	std::string serial = dto::marshall(*dto, resp.size());
	no_err(sendto(fd, &serial[0], serial.size(), 0, (struct sockaddr *) &current_client,
	              sockaddr_len), "Error in sendto");
	//TODO need to split into packets
}

void UDPServer::on_output(Poll &p) {
}

void on_msg(Poll &p, std::string message) {

}

UDPServer::~UDPServer() {
	//TODO unsubscribe multicast group
}

UDPServer::UDPServer(const std::string name, std::string addr, uint16_t port, std::shared_ptr<SharedDirectory> shdir,
                     int timeout)
	: Subscriber(name),
	  buffer(buf_len, '\0'),
	  dir(std::move(shdir)),
	  timeout(timeout),
	  port(port) {
	set_fd(connect_group(port, addr.c_str()));
	set_expected(POLLIN);
	std::cout << name << ": Listening on port " << port << std::endl;
}

