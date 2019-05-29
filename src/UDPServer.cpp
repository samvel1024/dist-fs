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
	struct sockaddr_in clientaddr;
	unsigned clientlen; /* byte size of client's address */
	clientlen = sizeof(clientaddr);
	int bytes_read = no_err(recvfrom(fd, &buffer[0], buffer.size(), 0,
	                                 (struct sockaddr *) &clientaddr, &clientlen), "Error in recvfrom");
	on_dispatch(p, bytes_read);
//	std::cout << name << "Server received:" << buffer << std::endl;
//	std::string resp(5000, 'A');
//	no_err(sendto(fd, &resp[0], resp.size(), 0, (struct sockaddr *) &clientaddr, clientlen), "Error in sendto");
}

void UDPServer::on_dispatch(Poll &p, int bytes_read) {
	std::string type_header = buffer.substr(0, dto::CMD_TYPE_LEN);
	switch (dto::from_header(type_header)) {
		case dto::HELLO_REQ : {
			on_hello(p, *(dto::unmarshall<dto::Simple>(buffer, bytes_read)));
			break;
		}

		default : {
			std::cout << name << ": Illegal type header " << type_header << ", skipping" << std::endl;
			return;
		}
	}
}


void UDPServer::on_hello(Poll &p, dto::Simple &msg) {
	std::cout << name << ": msg " << msg << std::endl;
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
	  timeout(timeout) {
	set_fd(connect_group(port, addr.c_str()));
	set_expected(POLLIN);
	std::cout << name << ": Listening on port " << port << std::endl;
}

