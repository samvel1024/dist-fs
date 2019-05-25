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

#include "err.h"

#define BSIZE         1024
#define REPEAT_COUNT  30
#define BUFSIZE 1024


#define PORT 10001

int main(int argc, char *argv[]) {
	/* argumenty wywoĹania programu */
	char *multicast_dotted_address;
	in_port_t local_port;

	/* zmienne i struktury opisujÄce gniazda */
	int sock;
	struct sockaddr_in local_address, clientaddr;
	unsigned int remote_len;
	struct ip_mreq ip_mreq;

	/* zmienne obsĹugujÄce komunikacjÄ */
	char buffer[BSIZE];
	ssize_t rcv_len;
	time_t time_buffer;
	int i;

	/* parsowanie argumentĂłw programu */
	if (argc != 3)
		fatal("Usage: %s multicast_dotted_address local_port\n", argv[0]);
	multicast_dotted_address = argv[1];
	local_port = (in_port_t) atoi(argv[2]);

	/* otworzenie gniazda */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		syserr("socket");
	u_int yes = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char *) &yes, sizeof(yes)) < 0) {
		perror("Reusing ADDR failed");
		return 1;
	}
	/* podpiÄcie siÄ do grupy rozsyĹania (ang. multicast) */
	ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (inet_aton(multicast_dotted_address, &ip_mreq.imr_multiaddr) == 0)
		syserr("inet_aton");
	if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &ip_mreq, sizeof ip_mreq) < 0)
		syserr("setsockopt");

	/* podpiÄcie siÄ pod lokalny adres i port */
	local_address.sin_family = AF_INET;
	local_address.sin_addr.s_addr = htonl(INADDR_ANY);
	local_address.sin_port = htons(local_port);
	if (bind(sock, (struct sockaddr *) &local_address, sizeof local_address) < 0)
		syserr("bind");
	struct hostent *hostp; /* client host info */


	unsigned clientlen; /* byte size of client's address */
	char buf[BUFSIZE]; /* message buf */
	char *hostaddrp; /* dotted decimal host addr string */
	int n; /* message byte size */
	clientlen = sizeof(clientaddr);
	for (int i = 0; i < 3; ++i) {

		/*
		 * recvfrom: receive a UDP datagram from a client
		 */
		bzero(buf, BUFSIZE);
		n = recvfrom(sock, buf, BUFSIZE, 0,
		             (struct sockaddr *) &clientaddr, &clientlen);
		if (n < 0)
			syserr("ERROR in recvfrom");

		/*
		 * gethostbyaddr: determine who sent the datagram
		 */
		printf("server received %lu/%d bytes: %s\n", strlen(buf), n, buf);

		/*
		 * sendto: echo the input back to the client
		 */
		char *bik = "PIZDEEC";
		n = sendto(sock, bik, strlen(bik), 0,
		           (struct sockaddr *) &clientaddr, clientlen);
		if (n < 0)
			syserr("ERROR in sendto");
	}
	/* w taki sposĂłb moĹźna odpiÄÄ siÄ od grupy rozsyĹania */
	if (setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void *) &ip_mreq, sizeof ip_mreq) < 0)
		syserr("setsockopt");

	/* koniec */
	close(sock);
	exit(EXIT_SUCCESS);
}

// fcntl(sd, F_SETFL, O_NONBLOCK); 