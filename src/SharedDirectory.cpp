//
// Created by Samvel Abrahamyan on 2019-05-29.
//

#include "SharedDirectory.h"
#include "nio/Error.h"

using namespace boost::filesystem;


SharedDirectory::SharedDirectory(const std::string &p, long sp) : shared_dir(p), total_space(sp) {
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
	long actual_space = 0;
	indexed_files.clear();
	for (directory_iterator itr{shared_dir}; itr != end; ++itr) {
		auto file = itr->path();
		if (!is_regular_file(itr->path())) {
			continue;
		}
		long size = file_size(file);
		actual_space += size;
		indexed_files.emplace(std::string(file.filename().string()));
	}
	remaining_space = total_space - actual_space;
}

std::set<std::string> SharedDirectory::get_files() {
	return indexed_files;
}

long SharedDirectory::get_remaining_space() const {
	return remaining_space;
}


void SharedDirectory::reserve_file(std::string name, long size) {
	if (!can_create_file(size, name)) {
		throw Error("Too big file");
	}
	pending_files[name] = size;
	remaining_space -= size;
}


bool SharedDirectory::can_create_file(long size, std::string name) {
	return size <= this->remaining_space && size > 0
	       && portable_name(name)
	       && (indexed_files.find(name) == indexed_files.end())
	       && (pending_files.find(name) == pending_files.end());
}

path SharedDirectory::path_in_dir(std::string p){
	path file_path(shared_dir);
	file_path /= p;
	return file_path;
}

void SharedDirectory::cancel_reserved_file(std::string f) {
	auto it = pending_files.find(f);
	if (it == pending_files.end()) {
		throw Error("File was not even reserved");
	}
	remaining_space += it->second;
	pending_files.erase(it);
}

boost::filesystem::fstream SharedDirectory::open_writable_file(std::string name) {
	if (pending_files.find(name) == pending_files.end()){
		throw Error("File should be reserved first");
	}
	return fstream(path_in_dir(name), std::ios::out);
}

boost::filesystem::fstream SharedDirectory::open_readable_file(std::string name) {
	if (!can_read_file(name)){
		throw Error("File does not exist");
	}
	return fstream(path_in_dir(name), std::ios::in);
}

bool SharedDirectory::can_read_file(std::string name) {
	return exists(path_in_dir(name)) && indexed_files.find(name) != indexed_files.end();
}

void SharedDirectory::on_finished_writing(std::string name) {
	auto it = pending_files.find(name);
	if (it == pending_files.end()){
		throw Error("Not registered file");
	}
	long promised_size = it->second;
	pending_files.erase(it);
	path p = path_in_dir(name);
	if(!exists(p)){
		throw Error("File not exists");
	}
	long size = file_size(p);
	if (size > promised_size){
		remove(p);
		remaining_space += promised_size;
	}
	else {
		remaining_space += promised_size - size;
		indexed_files.emplace(name);
	}
}

void SharedDirectory::delete_file(std::string name) {
	if (!can_read_file(name)){
		throw Error("The file is not deletable");
	}
	path p = path_in_dir(name);
	if (!exists(p)){
		throw Error("The file does not exist");
	}

	long size = file_size(p);
	indexed_files.erase(indexed_files.find(name));
	remaining_space += size;
	remove(p);
}







