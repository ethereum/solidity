/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0

#include <stdexcept>
#include <iostream>
#include <test/Common.h>

#include <libsolutil/Assertions.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace solidity::test
{

/// If non-empty returns the value of the env. variable ETH_TEST_PATH, otherwise
/// it tries to find a path that contains the directories "libsolidity/syntaxTests"
/// and returns it if found.
/// The routine searches in the current directory, and inside the "test" directory
/// starting from the current directory and up to three levels up.
/// @returns the path of the first match or an empty path if not found.
boost::filesystem::path testPath()
{
	if (auto path = getenv("ETH_TEST_PATH"))
		return path;

	auto const searchPath =
	{
		fs::current_path() / ".." / ".." / ".." / "test",
		fs::current_path() / ".." / ".." / "test",
		fs::current_path() / ".." / "test",
		fs::current_path() / "test",
		fs::current_path()
	};
	for (auto const& basePath: searchPath)
	{
		fs::path syntaxTestPath = basePath / "libsolidity" / "syntaxTests";
		if (fs::exists(syntaxTestPath) && fs::is_directory(syntaxTestPath))
			return basePath;
	}
	return {};
}

std::string envOrDefaultPath(std::string const& env_name, std::string const& lib_name)
{
	if (auto path = getenv(env_name.c_str()))
		return path;

	auto const searchPath =
	{
		fs::path("/usr/local/lib"),
		fs::path("/usr/lib"),
		fs::current_path() / "deps",
		fs::current_path() / "deps" / "lib",
		fs::current_path() / ".." / "deps",
		fs::current_path() / ".." / "deps" / "lib",
		fs::current_path() / ".." / ".." / "deps",
		fs::current_path() / ".." / ".." / "deps" / "lib",
		fs::current_path()
	};
	for (auto const& basePath: searchPath)
	{
		fs::path p = basePath / lib_name;
		if (fs::exists(p))
			return p.string();
	}
	return {};
}

CommonOptions::CommonOptions(std::string _caption):
	options(_caption,
		po::options_description::m_default_line_length,
		po::options_description::m_default_line_length - 23
	)
{
	options.add_options()
		("evm-version", po::value(&evmVersionString), "which evm version to use")
		("testpath", po::value<fs::path>(&this->testPath)->default_value(solidity::test::testPath()), "path to test files")
		("vm", po::value<std::vector<fs::path>>(&vmPaths), "path to evmc library, can be supplied multiple times.")
		("ewasm", po::bool_switch(&ewasm), "tries to automatically find an ewasm vm and enable ewasm test-execution.")
		("no-smt", po::bool_switch(&disableSMT), "disable SMT checker")
		("optimize", po::bool_switch(&optimize), "enables optimization")
		("enforce-via-yul", po::bool_switch(&enforceViaYul), "Enforce compiling all tests via yul to see if additional tests can be activated.")
		("abiencoderv1", po::bool_switch(&useABIEncoderV1), "enables abi encoder v1")
		("show-messages", po::bool_switch(&showMessages), "enables message output")
		("show-metadata", po::bool_switch(&showMetadata), "enables metadata output");
}

void CommonOptions::validate() const
{
	assertThrow(
		!testPath.empty(),
		ConfigException,
		"No test path specified. The --testpath argument must not be empty when given."
	);
	assertThrow(
		fs::exists(testPath),
		ConfigException,
		"Invalid test path specified."
	);

}

bool CommonOptions::parse(int argc, char const* const* argv)
{
	po::variables_map arguments;

	po::command_line_parser cmdLineParser(argc, argv);
	cmdLineParser.options(options);
	auto parsedOptions = cmdLineParser.run();
	po::store(parsedOptions, arguments);
	po::notify(arguments);

	for (auto const& parsedOption: parsedOptions.options)
		if (parsedOption.position_key >= 0)
		{
			if (
				parsedOption.original_tokens.empty() ||
				(parsedOption.original_tokens.size() == 1 && parsedOption.original_tokens.front().empty())
			)
				continue; // ignore empty options
			std::stringstream errorMessage;
			errorMessage << "Unrecognized option: ";
			for (auto const& token: parsedOption.original_tokens)
				errorMessage << token;
			throw std::runtime_error(errorMessage.str());
		}

	if (vmPaths.empty())
	{
		std::string evmone = envOrDefaultPath("ETH_EVMONE", evmoneFilename);
		if (!evmone.empty())
			vmPaths.emplace_back(evmone);
		else
		{
			std::cout << "Unable to find " << solidity::test::evmoneFilename
				 << ". Please provide the path using --vm <path>." << std::endl;
			std::cout << "You can download it at" << std::endl;
			std::cout << solidity::test::evmoneDownloadLink << std::endl;
		}
	}

	if (ewasm) {
		std::string hera = envOrDefaultPath("ETH_HERA", heraFilename);
		if (!hera.empty())
			vmPaths.emplace_back(hera);
		else {
			std::cout << "Unable to find " << solidity::test::heraFilename
					  << ". Please provide the path using --vm <path>." << std::endl;
			std::cout << "You can download it at" << std::endl;
			std::cout << solidity::test::heraDownloadLink << std::endl;
			std::cout << "Ewasm tests disabled." << std::endl;
		}
	}

	return true;
}


langutil::EVMVersion CommonOptions::evmVersion() const
{
	if (!evmVersionString.empty())
	{
		auto version = langutil::EVMVersion::fromString(evmVersionString);
		if (!version)
			throw std::runtime_error("Invalid EVM version: " + evmVersionString);
		return *version;
	}
	else
		return langutil::EVMVersion();
}


CommonOptions const& CommonOptions::get()
{
	if (!m_singleton)
		throw std::runtime_error("Options not yet constructed!");

	return *m_singleton;
}

void CommonOptions::setSingleton(std::unique_ptr<CommonOptions const>&& _instance)
{
	m_singleton = std::move(_instance);
}

std::unique_ptr<CommonOptions const> CommonOptions::m_singleton = nullptr;

}
