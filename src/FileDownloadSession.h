//
// Created by Samvel Abrahamyan on 2019-05-27.
//

#ifndef DISTFS_FILEDOWNLOADSESSION_H
#define DISTFS_FILEDOWNLOADSESSION_H

#include "nio/TCPServer.h"
#include "nio/TCPSessionFactory.h"

class FileDownloadSession : public Subscriber {

private:
	static constexpr int BUFLEN = 10000;
	char buffer[BUFLEN]{};
	int buf_sent{};
	int buf_size{};

public:

	void on_input(Poll &p) override;

	void on_output(Poll &p) override;

	~FileDownloadSession() override = default;

	explicit FileDownloadSession(std::string nm, int fd);
};


class FTSessionFactory : public TCPSessionFactory {
public:
	std::shared_ptr<Subscriber> create_session(TCPServer &serv, int fd) override;
	explicit FTSessionFactory();
	~FTSessionFactory() override;
};




#endif //DISTFS_FILEDOWNLOADSESSION_H
