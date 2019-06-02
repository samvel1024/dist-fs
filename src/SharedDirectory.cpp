//
// Created by Samvel Abrahamyan on 2019-05-29.
//

#include "SharedDirectory.h"
#include "nio/Error.h"

using namespace boost::filesystem;

SharedDirectory::SharedDirectory(const std::string &p, uint64_t sp) : shared_dir(p), total_space(sp) {
  if (!exists(p)) {
    if (!create_directory(p)) {
      throw Error("Could not create the directory");
    }
  }
  file_status s = status(shared_dir);
  if (!is_directory(s)) {
    throw Error("The file is not directory");
  }
  index_files();
}

void SharedDirectory::index_files() {
  directory_iterator end;
  uint64_t actual_space = 0;
  indexed_files.clear();
  for (directory_iterator itr{shared_dir}; itr != end; ++itr) {
    auto file = itr->path();
    if (!is_regular_file(itr->path())) {
      continue;
    }
    uint64_t size = file_size(file);
    actual_space += size;
    indexed_files.emplace(file.filename().string(), size);
  }
  remaining_space = total_space < actual_space ? 0 : total_space - actual_space;
}

std::map<std::string, uint64_t> SharedDirectory::get_files() {
  return indexed_files;
}

uint64_t SharedDirectory::get_remaining_space() const {
  return remaining_space;
}

void SharedDirectory::reserve_file(const std::string &name, uint64_t size) {
  if (!can_create_file(size, name)) {
    throw Error("Too big file");
  }
  pending_files[name] = size;
  remaining_space -= size;
}

bool SharedDirectory::can_create_file(uint64_t size, const std::string &name) {
  return size <= this->remaining_space && size > 0
      && portable_name(name)
      && !exists(path_in_dir(name))
      && (indexed_files.find(name) == indexed_files.end())
      && (pending_files.find(name) == pending_files.end());
}

path SharedDirectory::path_in_dir(std::string p) {
  path file_path(shared_dir);
  file_path /= p;
  return file_path;
}

void SharedDirectory::cancel_reserved_file(const std::string &f) {
  auto it = pending_files.find(f);
  if (it == pending_files.end()) {
    throw Error("File was not reserved");
  }
  remaining_space += it->second;
  pending_files.erase(it);
}

boost::filesystem::fstream SharedDirectory::open_writable_file(const std::string &name) {
  if (pending_files.find(name) == pending_files.end()) {
    throw Error("File should be reserved first");
  }
  return fstream(path_in_dir(name), std::ios::out);
}

boost::filesystem::fstream SharedDirectory::open_readable_file(const std::string &name) {
  if (!can_read_file(name)) {
    throw Error("File does not exist");
  }
  return fstream(path_in_dir(name), std::ios::in);
}

bool SharedDirectory::can_read_file(const std::string &name) {
  return exists(path_in_dir(name)) && indexed_files.find(name) != indexed_files.end();
}

void SharedDirectory::on_finished_writing(std::string name) {
  auto it = pending_files.find(name);
  if (it == pending_files.end()) {
    throw Error("Not registered file");
  }
  uint64_t promised_size = it->second;
  pending_files.erase(it);
  path p = path_in_dir(name);
  if (!exists(p)) {
    throw Error("File not exists");
  }
  uint64_t size = file_size(p);
  if (size > promised_size) {
    remove(p);
    remaining_space += promised_size;
  } else {
    remaining_space += promised_size - size;
    indexed_files.emplace(name, size);
  }
}

void SharedDirectory::delete_file(const std::string &name) {
  if (!can_read_file(name)) {
    throw Error("The file is not deletable");
  }
  path p = path_in_dir(name);
  if (!exists(p)) {
    throw Error("The file does not exist");
  }

  uint64_t size = file_size(p);
  indexed_files.erase(indexed_files.find(name));
  remaining_space += size;
  remove(p);
}

std::vector<std::string> SharedDirectory::search_file(const std::string &query) {
  std::vector<std::string> ans;
  for (auto &it:indexed_files) {
    if (query.empty() || (it.first.find(query) != std::string::npos && it.second > 0)) {
      ans.push_back(it.first);
    }
  }
  return ans;
}






