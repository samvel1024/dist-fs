#ifndef DISTFS_CLILISTENER_H
#define DISTFS_CLILISTENER_H

#include "nio/Subscriber.h"
#include "Dto.h"
#include <boost/filesystem.hpp>


class CLIListener : public Subscriber {
 private:
  uint64_t cmd_seq;
  uint16_t port;
  boost::filesystem::path out_dir;
  int timeout_sec;
  std::string mcast_addr;
  std::unordered_map<std::string, int> commands;
  std::unordered_map<std::string, std::string> file_server{};
  std::unordered_map<std::string, uint64_t> server_space{};

  void exec_command(Poll &p, std::string &type, std::string &arg);

 public:

  CLIListener(uint16_t port, boost::filesystem::path out_dir, int timeout_sec,
              std::string mcast_addr);

  void on_input(Poll &p) override;

  void on_error(Poll &p, int event) override;

  void do_discover(Poll &p);

  void do_search(Poll &p, std::string &str);

  void unblock_input(Poll &p);

  void block_input(Poll &p);
  void do_fetch(Poll &p, std::string &arg);
  void on_search_result(dto::Simple &resp, sockaddr_in addr);
  void on_fetch_result(Poll &p, dto::Complex &resp, sockaddr_in addr, std::string serv, uint16_t port);
  void do_upload(Poll &poll, std::string &full_path);
  void on_discover_result(dto::Complex &resp, sockaddr_in addr);
  void do_delete(Poll &poll, std::string &basic_string);
  void on_upload_result(Poll &p,
                        dto::Complex &dto,
                        sockaddr_in ad,
                        boost::filesystem::path file,
                        std::vector<std::pair<uint64_t, std::string>> servs,
                        int last_tried);
  void do_upload_query(Poll &poll,
                       boost::filesystem::path file,
                       std::vector<std::pair<uint64_t, std::string>> servers,
                       int curr_server);
};

#endif //DISTFS_CLILISTENER_H
