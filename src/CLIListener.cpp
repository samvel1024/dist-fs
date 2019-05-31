#include <utility>

#include <utility>

//
// Created by Samvel Abrahamyan on 2019-05-30.
//

#include "CLIListener.h"
#include <boost/algorithm/string.hpp>
#include "Dto.h"
#include <algorithm>
#include "MultiQuery.h"


// TODO the input should come from a pipe opened at other thread, or read with one byte buffer
void CLIListener::on_input(Poll &p) {
	char buf[10000];
	int r = read(fd, buf, 10000);
	buf[r - 1] = '\0'; // The last one should be new line char

	std::string line(buf);
	std::vector<std::string> tokens;
	boost::split(tokens, line, boost::is_any_of(" "));
	if (tokens.empty() || tokens.size() > 2) {
		std::cout << name << " :Illegal input, ignoring" << std::endl;
		return;
	}
	std::transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(), ::tolower);
	std::string arg = tokens.size() == 2 ? tokens[1] : std::string();
	exec_command(p, tokens[0], arg);
}

void CLIListener::exec_command(Poll &p, std::string &type, std::string &arg) {
	int t = commands.find(type) == commands.end() ? -2 : commands[type];
	switch (t) {
		case -2: {
			std::cout << name << " :Illegal input, ignoring" << std::endl;
			break;
		}
		case -1: {
			p.do_shutdown();
			break;
		}
		case dto::Type::HELLO_REQ : {
			this->do_discover(p);
			break;
		}
		case dto::Type::LIST_REQ: {
			this->do_search(p, arg);
			break;
		}
		default : {
			throw Error("This should not happen");
		}
	}
}

void CLIListener::do_search(Poll &p, std::string &arg) {
	this->set_expected(0);
	p.notify_subscriber_changed(*this);
	std::shared_ptr<MultiQuery> query = std::make_shared<MultiQuery>(port, mcast_addr, timeout_sec * 1000);
	p.subscribe(query);
	uint64_t seq = cmd_seq++;
	auto dto = dto::create_dto<dto::Simple>(arg.size(), seq);
	dto::init_common(*dto, arg, "LIST");
	std::string pld = dto::marshall(*dto, arg.size());
	CLIListener *self = this;
	query->execute(
		pld,
		[seq](std::string &resp, sockaddr_in addr) -> void {
			auto resp_dto = dto::unmarshall<dto::Simple>(resp, resp.size());
			if (resp_dto->cmd_seq != seq) {
				throw Error("Illegal packet, cmd_seq, expected %llu got %llu", seq, resp_dto->cmd_seq);
			}
			std::cout << "Resp: " << resp_dto->payload << std::endl;
		},
		[&p, self]() {
			self->set_expected(POLLIN);
			p.notify_subscriber_changed(*self);
		},
		[&p, self]() {
			self->set_expected(POLLIN);
			p.notify_subscriber_changed(*self);
		});
}

void CLIListener::do_discover(Poll &p) {
	this->set_expected(0);
	p.notify_subscriber_changed(*this);

	auto query = std::make_shared<MultiQuery>(port, mcast_addr, timeout_sec * 1000);
	p.subscribe(query);

	auto seq = cmd_seq++;
	auto dto = dto::create_dto<dto::Simple>(0, seq);
	std::string empty;
	dto::init(*dto, empty, "HELLO");
	auto pld = dto::marshall(*dto, 0);
	CLIListener *self = this;
	query->execute(
		pld,
		[seq](std::string &resp, sockaddr_in addr) -> void {
			auto resp_dto = dto::unmarshall<dto::Complex>(resp, resp.size());
			if (resp_dto->cmd_seq != seq) {
				throw Error("Ilegal packet ..");
			}
			std::cout << "Resp: " << resp_dto->payload << std::endl;
		},
		[&p, self]() {
			self->set_expected(POLLIN);
			p.notify_subscriber_changed(*self);
		},
		[&p, self]() {
			self->set_expected(POLLIN);
			p.notify_subscriber_changed(*self);
		});


}


void CLIListener::on_error(Poll &p, int event) {
	if (event & POLLHUP) {
		std::cout << name << ": Ignoring POLLHUP" << std::endl;
		return;
	}
	Subscriber::on_error(p, event);
}

CLIListener::CLIListener(uint16_t port, fs::path out_dir, int timeout_sec, std::string mcast_addr) :
	Subscriber("CLIListener"), cmd_seq(0), port(port),
	out_dir(std::move(out_dir)), timeout_sec(timeout_sec),
	mcast_addr(std::move(mcast_addr)) {
	set_fd(STDIN_FILENO);
	set_expected(POLLIN);
	commands = std::unordered_map<std::string, int>{
		{"discover", dto::Type::HELLO_REQ},
		{"search",   dto::Type::LIST_REQ},
		{"fetch",    dto::Type::DOWNLOAD_REQ},
		{"upload",   dto::Type::UPLOAD_REQ},
		{"remove",   dto::Type::DEL_REQ},
		{"exit",     -1} //Hacky way
	};
}


