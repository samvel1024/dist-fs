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
#include "FileReceiveSession.h"
#include "FileSendSession.h"

using namespace std::placeholders;
namespace fs = boost::filesystem;


void print_prompt() {
  std::cout << "ENTER A COMMAND:" << std::endl;
}

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
  if (this->expected & POLLIN){
    print_prompt();
  }
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
    case dto::Type::DOWNLOAD_REQ: {
      this->do_fetch(p, arg);
      break;
    }
    case dto::Type::UPLOAD_REQ: {
      this->do_upload(p, arg);
      break;
    }
    case dto::Type::DEL_REQ: {
      this->do_delete(p, arg);
      break;
    }
    default : {
      throw Error("This should not happen");
    }
  }
}



void CLIListener::block_input(Poll &p) {
  set_expected(0);
  p.notify_subscriber_changed(*this);
}

void CLIListener::unblock_input(Poll &p) {
  set_expected(POLLIN);
  p.notify_subscriber_changed(*this);
  print_prompt();
}

std::string get_ip(sockaddr_in addr) {
  std::string ip(INET_ADDRSTRLEN, '\0');
  inet_ntop(AF_INET, &(addr.sin_addr), &ip[0], INET_ADDRSTRLEN);
  return ip;
}

void request_failed() {
  std::cout << "Failed UDP request" << std::endl;
}

void CLIListener::on_search_result(dto::Simple &resp, sockaddr_in addr) {
  std::vector<std::string> tokens;
  boost::split(tokens, resp.payload, boost::is_any_of("\n"));
  std::string ip = get_ip(addr);
  //TODO take care of upload File Name.txt
  for (auto &f: tokens) {
    std::cout << f << " (" << ip << ")" << std::endl;
    this->file_server[f] = ip;
  }
}

void CLIListener::do_search(Poll &p, std::string &arg) {
  block_input(p);
  auto reqdto = dto::create(this->cmd_seq++, "LIST", arg);
  auto query = std::make_shared<MultiQuery<dto::Simple, dto::Simple>>(reqdto, port, mcast_addr, timeout_sec * 1000);
  p.subscribe(query);
  auto unblock = std::bind(&CLIListener::unblock_input, this, std::ref(p));
  query->when_response(std::bind(&CLIListener::on_search_result, this, _1, _2));
  query->when_error(unblock);
  query->when_timeout(unblock);
}

void CLIListener::on_discover_result(dto::Complex &resp, sockaddr_in addr) {
  std::string mcast = std::move(resp.payload);
  uint64_t disk = resp.header.param;
  std::cout << "Found " << get_ip(addr) << " (" << mcast << ") with free space " << disk << std::endl;
  server_space[get_ip(addr)] = resp.header.param;
}

void CLIListener::do_discover(Poll &p) {
  block_input(p);
  auto empty = std::string();
  auto dto = dto::create(cmd_seq++, "HELLO", empty);
  auto query = std::make_shared<MultiQuery<dto::Simple, dto::Complex>>(dto, port, mcast_addr, timeout_sec * 1000);
  p.subscribe(query);
  auto unblock = std::bind(&CLIListener::unblock_input, this, std::ref(p));
  query->when_response(std::bind(&CLIListener::on_discover_result, this, _1, _2));
  query->when_error(unblock);
  query->when_timeout(unblock);
}

int initialize_tcp(std::string host, std::string port) {
  struct addrinfo addr_hints;
  struct addrinfo *addr_result;
  memset(&addr_hints, 0, sizeof(struct addrinfo));
  addr_hints.ai_family = AF_INET; // IPv4
  addr_hints.ai_socktype = SOCK_STREAM;
  addr_hints.ai_protocol = IPPROTO_TCP;
  int err = getaddrinfo(&host[0], &port[0], &addr_hints, &addr_result);
  if (err == EAI_SYSTEM || err != 0) {
    fprintf(stderr, "getaddrinfo: %s", gai_strerror(err));
    throw Error("Failed to create tcp client socket");
  }
  int sock;
  no_err(sock = socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol), "tcp socket");
  no_err(connect(sock, addr_result->ai_addr, addr_result->ai_addrlen), "tcp connect");

  freeaddrinfo(addr_result);
  return sock;
}

void CLIListener::on_fetch_result(Poll &p, dto::Complex &resp, sockaddr_in addr, std::string serv, uint16_t prt) {
  int tcp_fd = initialize_tcp(get_ip(addr), std::to_string(resp.header.param));
  fs::path path(out_dir);
  path /= fs::path(resp.payload);
  auto session = std::make_shared<FileReceiveSession>(path, [serv, prt](fs::path path) {
    std::cout << "File " << path.filename().string() << " downloaded (" << serv << ":" << prt << ")" << std::endl;
  });
  int hct = 1;
  no_err(ioctl(tcp_fd, FIONBIO, (char *) &hct), "Setting to non blocking");
  session->set_fd(tcp_fd);
  p.subscribe(session);
}

void CLIListener::do_fetch(Poll &p, std::string &arg) {
  if (file_server.find(arg) == file_server.end()) {
    //TODO print message;
    return;
  }
  if (file_server.find(arg) == file_server.end()) {
    std::cout << "Not found file " << arg << " in search results" << std::endl;
    return;
  }
  auto addr = file_server[arg];
  auto dto = dto::create(cmd_seq++, "GET", arg);
  auto query = std::make_shared<MultiQuery<dto::Simple, dto::Complex>>(dto, port, addr, 10000);
  p.subscribe(query);
  query->when_response(std::bind(&CLIListener::on_fetch_result, this, std::ref(p), _1, _2, addr, port));
  query->when_error(request_failed);
}

std::string CLIListener::get_largest_server() {
  uint64_t space = 0;
  std::string ans;
  for (const auto &entry: server_space) {
    if (entry.second > space) {
      space = entry.second;
      ans = entry.first;
    }
  }
  return ans;
}

void CLIListener::on_upload_result(Poll &p, dto::Complex &dto, sockaddr_in ad, boost::filesystem::path file) {
  std::string hdr = dto.header.cmd;
  auto server = get_ip(ad);
  dto::Type t = dto::from_header(hdr);
  if (t != dto::Type::UPLOAD_RES_OK) { // Dirty way to avoid having custom deserialization in MultiQuery
    std::cout << "File " << file.string() << " uploading failed (" << server << ":" << ntohs(ad.sin_port)
              << ") refused to store the file" << std::endl;
    return;
  }
  int fd = initialize_tcp(&server[0], std::to_string(dto.header.param));
  auto session = std::make_shared<FileSendSession>(file);
  session->when_success([file, server, ad] {
    std::cout << "File " << file.string() << " uploaded  (" << server << ":" << ntohs(ad.sin_port) << ")" << std::endl;
  });
  session->set_fd(fd);
  p.subscribe(session);
}

void CLIListener::do_upload(Poll &poll, std::string &full_path) {
  fs::path path(full_path);
  if (!fs::exists(path) || !fs::is_regular_file(path)) {
    std::cout << "File " << full_path << " does not exist" << std::endl;
    return;
  }
  std::string server = get_largest_server();
  uint64_t size = fs::file_size(path);
  if (server.empty() || size > server_space[server]) {
    std::cout << "File " << full_path << " too big" << std::endl;
    return;
  }
  uint16_t prt = this->port;
  std::string filename = path.filename().string();
  auto dto = dto::create(this->cmd_seq++, "ADD", filename, size);
  auto query = std::make_shared<MultiQuery<dto::Complex, dto::Complex>>
      (dto, prt, server, 10000);
  poll.subscribe(query);
  query->when_response(std::bind(&CLIListener::on_upload_result, this, std::ref(poll), _1, _2, path));
  query->when_error([full_path, server, prt] {
    std::cout << "File " << full_path << " uploading failed (" << server << ":" << prt << ") Error in connection"
              << std::endl;
  });
}

void CLIListener::do_delete(Poll &poll, std::string &str) {
  if (str.empty()) {
    std::cout << "Cannot be empty" << std::endl;
    return;
  }
  auto dto = dto::create(cmd_seq++, "DEL", str);
  auto query = std::make_shared<MultiQuery<dto::Simple, dto::Simple>>(dto, port, mcast_addr, 0);
  poll.subscribe(query);
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
      {"search", dto::Type::LIST_REQ},
      {"fetch", dto::Type::DOWNLOAD_REQ},
      {"upload", dto::Type::UPLOAD_REQ},
      {"remove", dto::Type::DEL_REQ},
      {"exit", -1} //Hacky way
  };
  print_prompt();
}


