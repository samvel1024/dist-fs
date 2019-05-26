#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <string>
#include <iostream>
#include <poll.h>
#include "err.h"

#define BSIZE         1024
#define REPEAT_COUNT  30
#define BUFSIZE 1024


int TRY(int val, const char *msg) {
	if (val < 0) syserr(msg);
	return val;
}

int connect_group(in_port_t port, char *addr) {
	int sock = TRY(socket(AF_INET, SOCK_DGRAM, 0), "socket");
	u_int yes = 1;
	TRY(setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char *) &yes, sizeof(yes)), "reuse");
	//Connect to group
	struct ip_mreq ip_mreq{};
	ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	TRY(inet_aton(addr, &ip_mreq.imr_multiaddr) == 0, "inet_aton");
	TRY(setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &ip_mreq, sizeof ip_mreq), "setsockopt");
	//Bind to port
	struct sockaddr_in local_address{};
	local_address.sin_family = AF_INET;
	local_address.sin_addr.s_addr = htonl(INADDR_ANY);
	local_address.sin_port = htons(port);
	if (bind(sock, (struct sockaddr *) &local_address, sizeof local_address) < 0)
		syserr("bind");
	fcntl(sock, F_SETFL, O_NONBLOCK);
	return sock;
}


int main(int argc, char *argv[]) {
	if (argc != 3)
		fatal("Usage: %s multicast_dotted_address local_port\n", argv[0]);
	int sock = connect_group(atoi(argv[2]), argv[1]);
	struct sockaddr_in clientaddr;
	unsigned clientlen; /* byte size of client's address */
	std::string buf(BUFSIZE, '\0'); /* message buf */
	clientlen = sizeof(clientaddr);
	int pid = getpid();
	printf("[%d] Listening\n", pid);
	for (int i = 0; i < 2; ++i) {
		bzero(&buf[0], BUFSIZE);
		TRY(recvfrom(sock, &buf[0], BUFSIZE, 0,
		             (struct sockaddr *) &clientaddr, &clientlen), "Error in recvfrom");
		std::cout << "[" << pid << "]" << "Server received:" << buf << std::endl << std::endl;
		std::string resp = "HELLO WORLD";
		TRY(sendto(sock, &resp[0], resp.size(), 0, (struct sockaddr *) &clientaddr, clientlen), "Error in sendto");
	}
}
