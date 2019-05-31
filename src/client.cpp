#include <boost/program_options.hpp>
#include "nio/Poll.h"
#include "nio/Subscriber.h"
#include "CLIListener.h"
#include "nio/KillReceiver.h"
#include <boost/filesystem.hpp>


namespace po = boost::program_options;
using namespace boost::filesystem;

path create_out_folder(std::string st) {
	path p(st);
	if (!exists(p) && !create_directory(p)) {
		throw Error("Could not create output dir %s", &st[0]);
	}
	return p;
}


po::variables_map validate_args(int ac, char **av) {
	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "Help")
		("mcast,g", po::value<std::string>()->required(), "Multicast address")
		("udp-port,p", po::value<uint16_t>()->required(), "Servers UDP port")
		("shared-folder,o", po::value<std::string>()->required(), "Output folder")
		("timeout,t", po::value<int>()->default_value(5), "Wait timeout");
	po::variables_map vm;
	try {
		po::store(po::parse_command_line(ac, av, desc), vm);
		if (vm.count("help")) {
			std::cout << "File store client" << std::endl
			          << desc << std::endl;
			exit(0);
		}
		po::notify(vm);
	} catch (po::error &e) {
		std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
		std::cerr << desc << std::endl;
		exit(1);
	}
	create_out_folder(vm["shared-folder"].as<std::string>());
	int t = vm["timeout"].as<int>();
	if (t <= 0 || t > 300) {
		throw Error("Illegal timeout value");
	}
	return vm;
}


int main(int ac, char **av) {
	po::variables_map args = validate_args(ac, av);
	Poll poll;
	poll
		.subscribe(std::make_shared<CLIListener>(
			args["udp-port"].as<uint16_t>(),
			path(args["shared-folder"].as<std::string>()),
			args["timeout"].as<int>(),
			args["mcast"].as<std::string>()
		))
		.subscribe(std::make_shared<KillReceiver>());
	poll.loop();
}

