//
// Created by Samvel Abrahamyan on 2019-05-26.
//

#include "TCPServer.h"
#include "util.h"


void TCPSession::on_input(Poll &p) {

}

void TCPSession::on_output(Poll &p) {
	sprintf(this->buffer, "Hello World");
	write(this->get_fd(), this->buffer, strlen(this->buffer));

}


void TCPServer::on_input(Poll &p) {
	int connected_fd;
	do {
		struct sockaddr_in client;
		socklen_t client_len = sizeof(client);
		connected_fd = accept(this->get_fd(), (struct sockaddr *) &client, &client_len);
		if (connected_fd > 0) {
			std::shared_ptr<Subscriber> session = std::make_shared<TCPSession>();
			session->set_fd(connected_fd);
			session->set_expected(POLLOUT);
			p.subscribe(session);
			std::cout << "TCPServer: new connection" << std::endl;
		}
	} while (connected_fd > 0);
	if (connected_fd < 0 && errno != EWOULDBLOCK) {
		no_err(-1, "accept failed");
	}
}

void TCPServer::on_output(Poll &p) {

}


TCPServer::TCPServer(int port) {
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET; // IPv4
	server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
	server_address.sin_port = htons(port);
	int sock = no_err(socket(PF_INET, SOCK_STREAM, 0), "create socket"); // creating IPv4 TCP file_desc

	int yes = 1;
	no_err(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)), "setsockopt reuseaddr");
	no_err(bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)), "bind");
	// switch to listening (passive open)
	no_err(listen(sock, 5), "listen");
	set_fd(sock);
	set_expected(POLLIN);
	socklen_t len = sizeof(server_address);
	no_err(getsockname(sock, (struct sockaddr *) &server_address, &len), "getsockname");
	std::cout << "TCPServer: listening on port " << ntohs(server_address.sin_port) << std::endl;
}



