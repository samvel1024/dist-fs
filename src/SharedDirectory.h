//
// Created by Samvel Abrahamyan on 2019-05-29.
//

#ifndef DISTFS_SHAREDDIRECTORY_H
#define DISTFS_SHAREDDIRECTORY_H

#include <string>
#include <boost/filesystem.hpp>
#include <set>
#include <map>

class SharedDirectory {

 private:
  std::map<std::string, long> indexed_files;
  std::map<std::string, long> pending_files;
  boost::filesystem::path shared_dir;
  const long total_space;
  long remaining_space{};

 private:

  void index_files();

 public:

  boost::filesystem::path path_in_dir(std::string p);

  SharedDirectory(const std::string &p, long sp);

  std::map<std::string, long> get_files();

  long get_remaining_space() const;

  void reserve_file(const std::string &, long size);

  bool can_create_file(long size, const std::string &name);

  void cancel_reserved_file(const std::string &f);

  boost::filesystem::fstream open_writable_file(const std::string &name);

  void on_finished_writing(std::string name);

  bool can_read_file(const std::string &name);

  boost::filesystem::fstream open_readable_file(const std::string &name);

  void delete_file(const std::string &name);

  std::string search_file(const std::string &query);
};

#endif //DISTFS_SHAREDDIRECTORY_H
