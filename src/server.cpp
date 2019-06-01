#include <boost/program_options.hpp>
#include <iostream>

#include "nio/Poll.h"
#include "UDPServer.h"
#include "nio/KillReceiver.h"
#include "SharedDirectory.h"
#include <memory>

namespace po = boost::program_options;

po::variables_map validate_args(int ac, char **av) {
  boost::program_options::options_description desc("Allowed options");
  desc.add_options()
      ("help,h", "Help")
      ("mcast,g", po::value<std::string>()->required(), "Multicast address")
      ("udp-port,p", po::value<uint16_t>()->required(), "UDP port")
      ("max-space,b", po::value<long>()->default_value(52428800), "Max space in folder")
      ("shared-folder,f", po::value<std::string>()->required(), "Shared folder")
      ("timeout,t", po::value<int>()->default_value(5), "Wait timeout");
  po::variables_map vm;
  try {
    po::store(po::parse_command_line(ac, av, desc), vm);
    if (vm.count("help")) {
      std::cout << "File store server" << std::endl
                << desc << std::endl;
      exit(0);
    }
    po::notify(vm);
  } catch (po::error &e) {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
    std::cerr << desc << std::endl;
    exit(1);
  }
  return vm;
}

int main(int ac, char **av) {
  po::variables_map arg = validate_args(ac, av);
  auto shdir = std::make_shared<SharedDirectory>(arg["shared-folder"].as<std::string>(), arg["max-space"].as<long>());
  Poll poll;
  poll
      .subscribe(std::make_shared<UDPServer>(
          std::string("UDPServer"),
          arg["mcast"].as<std::string>(),
          arg["udp-port"].as<uint16_t>(),
          shdir,
          arg["timeout"].as<int>()))
      .subscribe(std::make_shared<KillReceiver>());
  poll.loop();
  return 0;
}