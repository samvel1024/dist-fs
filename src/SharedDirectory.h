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
	std::set<std::string> indexed_files;
	std::map<std::string, long> pending_files;
	boost::filesystem::path shared_dir;
	const long total_space;
	long remaining_space;

private:

	void index_files();

	boost::filesystem::path path_in_dir(std::string p);

public:

	SharedDirectory(const std::string &p, long sp);

	std::set<std::string> get_files();

	long get_remaining_space() const;

	void reserve_file(std::string, long size);

	bool can_create_file(long size, std::string name);

	void cancel_reserved_file(std::string f);

	boost::filesystem::fstream open_writable_file(std::string name);

	void on_finished_writing(std::string name);

	bool can_read_file(std::string name);

	boost::filesystem::fstream open_readable_file(std::string name);

	void delete_file(std::string name);
};


#endif //DISTFS_SHAREDDIRECTORY_H
