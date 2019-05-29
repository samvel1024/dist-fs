#define BOOST_TEST_MODULE SharedDirTests

#include <boost/test/unit_test.hpp>

#include "../src/SharedDirectory.h"
#include "../src/nio/Error.h"
using namespace boost::filesystem;

#define TESTDIR "__mtest_dir__"

class TempDir {
	path p;
public:
	TempDir(path pt) : p(pt) {
		create_directory(p);
	}

	virtual ~TempDir() {
		remove_all(p);
		BOOST_ASSERT(!exists(p));
	}
};

BOOST_AUTO_TEST_CASE(TestIllegalFiles) {
	TempDir(TESTDIR);
	SharedDirectory dir(TESTDIR, 100000);
	BOOST_ASSERT(dir.can_create_file(10, "arzati"));
	BOOST_ASSERT(!dir.can_create_file(10, "arzati/babik"));
	BOOST_ASSERT(!dir.can_create_file(10, "../arzati"));
	BOOST_ASSERT(dir.can_create_file(100000, "arzati"));
	BOOST_ASSERT(!dir.can_create_file(100001, "arzati"));
}

BOOST_AUTO_TEST_CASE(TestReservation) {
	TempDir(TESTDIR);
	const int size = 100000;
	SharedDirectory dir(TESTDIR, size);
	BOOST_ASSERT(dir.can_create_file(20, "file"));
	dir.reserve_file("file", 20);
	BOOST_ASSERT(!dir.can_create_file(10, "file"));
	BOOST_CHECK_EQUAL(dir.get_remaining_space(), size - 20);
	dir.cancel_reserved_file("file");
	BOOST_CHECK_EQUAL(dir.get_remaining_space(), size);
	BOOST_ASSERT(dir.can_create_file(10, "file"));
}

BOOST_AUTO_TEST_CASE(TestFileCreation) {
	TempDir(TESTDIR);
	const int dsize = 100000;
	const int fsize = 100;
	const std::string fname = "flpehsn";
	SharedDirectory dir(TESTDIR, dsize);
	BOOST_ASSERT(dir.can_create_file(fsize, fname));
	dir.reserve_file(fname, fsize);
	fstream write = dir.open_writable_file(fname);
	std::string written = std::string(fsize - 10, 'A');
	write << written;
	write.close();

	dir.on_finished_writing(fname);
	auto fileset = dir.get_files();
	BOOST_ASSERT(fileset.find(fname) != fileset.end());
	BOOST_ASSERT(dir.can_read_file(fname));
	BOOST_CHECK_EQUAL(dir.get_remaining_space(), dsize - fsize + 10);
	fstream read_file = dir.open_readable_file(fname);
	std::string data;
	read_file >> data;
	BOOST_CHECK_EQUAL(data, written);

	dir.delete_file(fname);
	auto fs = dir.get_files();
	BOOST_ASSERT(fs.find(fname) == fs.end());
	BOOST_ASSERT(dir.get_remaining_space() == dsize);
	BOOST_ASSERT(dir.can_create_file(10, fname));
	BOOST_ASSERT(!dir.can_read_file(fname));
}

