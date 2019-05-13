#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include "options.h"

namespace po = boost::program_options;

void Options::Parse(int argc, char** argv, const std::vector<std::string> preset_names)
{
	po::options_description description("Ethereum VM fuzzer usage");
	description.add_options()
			("help,h", "Display this help message")
			("trace", po::bool_switch()->default_value(false), "Show VM trace")
			("cmp_success", po::bool_switch()->default_value(false), "Compare VM return value and break on mismatch")
			("cmp_trace", po::bool_switch()->default_value(false), "Compare instruction traces and break on mismatch")
			("cmp_stack", po::bool_switch()->default_value(false), "Compare stack states and break on mismatch")
			("cmp_gas", po::bool_switch()->default_value(false), "Compare gas traces and break on mismatch")
			("cmp_msize", po::bool_switch()->default_value(false), "Compare memory sizes and break on mismatch")
			("diversity_guided", po::bool_switch()->default_value(false),
			 "(experimental) gravitate towards inputs with a diverse mix of instructions")
			("validate_code", po::bool_switch()->default_value(false),
			 "Reject inputs that do not constitute validate bytecode")
			("no_prestate", po::bool_switch()->default_value(false), "Do not extract a prestate from the input")
			("preset", po::value<std::string>()->default_value(""), "Select a preset")
			("gas", po::value<size_t>()->default_value(0),
			 "Set a fixed initial gas amount for all inputs. 0 = don't override")
			("max_gas", po::value<size_t>()->default_value(100000), "Place an upper bound for initial gas amount")
			("balance", po::value<size_t>()->default_value(0),
			 "Set a fixed initial balance for all inputs. 0 = don't override")
			("max_balance", po::value<size_t>()->default_value(100), "Place an upper bound on initial balance")
			("min_code_size", po::value<size_t>()->default_value(10), "Minimum size of the bytecode in bytes")
			("hardfork", po::value<std::string>()->default_value("byzantium"),
			 "Hardfork. Valid values are: "
			 "homestead, daofork, eip150, eip155, byzantium, constantinople, petersburg")

			("from_statetest", po::value<std::string>()->default_value(""),
			 "Convert statetest to fuzzer input. Usage: from_statetest=infile,outfile")
			("to_statetest", po::value<std::string>()->default_value(""),
			 "Convert fuzzer input to statetest. Usage: to_statetest=outfile")

			/* Geth-specific */
			("no_geth", po::bool_switch()->default_value(false), "Disable Geth")
			("merge", po::bool_switch()->default_value(false), "Merge mode (use with -merge)")
			("geth_inst_type", po::value<std::string>()->default_value("code"),
			 "Geth instrumentation type. Valid values are: code (default), stack, heap, intensity")
			("geth_symcov", po::value<std::string>()->default_value(""),
			 "Create .symcov file of Geth code coverage. Usage: geth_symcov=outfile")
			("geth_minimize", po::value<std::string>()->default_value(""),
			 "Minimize corpus: Usage: geth_symcov=incorpus,outcorpus")

			/* Parity-specific */
			("no_parity", po::bool_switch()->default_value(false), "Disable Parity")

			/* CPP-specific */
			("no_cpp", po::bool_switch()->default_value(false), "Disable cpp-ethereum");

	po::variables_map vm;
	try {
		po::store(po::command_line_parser(argc, argv).options(description).allow_unregistered().run(), vm);
	} catch (po::error& e) {
		std::cout << "Error parsing command-line options: " << e.what() << std::endl << std::endl;
		std::cout << description;
		exit(0);
	}
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << description;
		exit(0);
	}

	this->trace = vm["trace"].as<bool>();
	this->cmp_success = vm["cmp_success"].as<bool>();
	this->cmp_trace = vm["cmp_trace"].as<bool>();
	this->cmp_stack = vm["cmp_stack"].as<bool>();
	this->cmp_gas = vm["cmp_gas"].as<bool>();
	this->cmp_msize = vm["cmp_msize"].as<bool>();
	this->diversity_guided = vm["diversity_guided"].as<bool>();
	this->validate_code = vm["validate_code"].as<bool>();
	this->no_prestate = vm["no_prestate"].as<bool>();

	this->preset = vm["preset"].as<std::string>();
	if (this->preset != std::string("")) {
		bool found = false;
		for (const auto& pn : preset_names) {
			if (this->preset == pn) {
				found = true;
				break;
			}
		}

		if (found == false) {
			std::cout << "Preset " << this->preset << " not available" << std::endl;
			std::cout << "Valid presets are : " << std::endl;
			for (const auto& pn : preset_names) {
				std::cout << "    " << pn << std::endl;
			}
			exit(0);
		}
	}

	this->gas = vm["gas"].as<size_t>();
	this->max_gas = vm["max_gas"].as<size_t>();
	this->balance = vm["balance"].as<size_t>();
	this->max_balance = vm["max_balance"].as<size_t>();
	this->min_code_size = vm["min_code_size"].as<size_t>();

	{
		const auto hardfork = vm["hardfork"].as<std::string>();
		if (hardfork == "homestead") {
			blocknum = 1150000;
		} else if (hardfork == "daofork") {
			blocknum = 1920000;
		} else if (hardfork == "eip150") {
			blocknum = 2463000;
		} else if (hardfork == "eip155") {
			blocknum = 2675000;
		} else if (hardfork == "byzantium") {
			blocknum = 4370000;
		} else if (hardfork == "constantinople") {
			blocknum = 7280000;
		} else if (hardfork == "petersburg") {
			blocknum = 7280000;
		} else {
			std::cout << "Invalid hardfork selected" << std::endl;
			exit(0);
		}
	}

	if (vm["from_statetest"].as<std::string>().size()) {
		const auto fromStatetest = vm["from_statetest"].as<std::string>();
		std::vector<std::string> strs;
		boost::split(strs, fromStatetest, boost::is_any_of(","));

		if (strs.size() != 2) {
			std::cout << description;
			exit(0);
		}

		auto inputPath = strs[0];
		auto outputPath = strs[1];
		boost::algorithm::trim(inputPath);
		boost::algorithm::trim(outputPath);

		this->from_statetest_in = inputPath;
		this->from_statetest_out = outputPath;
	}

	if (vm["to_statetest"].as<std::string>().size()) {
		this->to_statetest_out = vm["to_statetest"].as<std::string>();
	}

	this->no_geth = vm["no_geth"].as<bool>();
	this->merge = vm["merge"].as<bool>();
	this->geth_inst_type = vm["geth_inst_type"].as<std::string>();
	if (this->geth_inst_type != std::string("code") &&
	    this->geth_inst_type != std::string("stack") &&
	    this->geth_inst_type != std::string("heap") &&
	    this->geth_inst_type != std::string("intensity")) {
		std::cout << description;
		exit(0);
	}

	if (vm["geth_symcov"].as<std::string>().size()) {
		const auto fromStatetest = vm["geth_symcov"].as<std::string>();
		std::vector<std::string> strs;
		boost::split(strs, fromStatetest, boost::is_any_of(","));

		if (strs.size() != 2) {
			std::cout << description;
			exit(0);
		}

		auto inputPath = strs[0];
		auto outputPath = strs[1];
		boost::algorithm::trim(inputPath);
		boost::algorithm::trim(outputPath);

		this->geth_symcov_in = inputPath;
		this->geth_symcov_out = outputPath;
	}

	if (vm["geth_minimize"].as<std::string>().size()) {
		const auto fromStatetest = vm["geth_minimize"].as<std::string>();
		std::vector<std::string> strs;
		boost::split(strs, fromStatetest, boost::is_any_of(","));

		if (strs.size() != 2) {
			std::cout << description;
			exit(0);
		}

		auto inputPath = strs[0];
		auto outputPath = strs[1];
		boost::algorithm::trim(inputPath);
		boost::algorithm::trim(outputPath);

		this->geth_minimize_in = inputPath;
		this->geth_minimize_out = outputPath;
	}

	this->no_parity = vm["no_parity"].as<bool>();

	this->no_cpp = vm["no_cpp"].as<bool>();
}
