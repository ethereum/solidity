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

#include <test/Common.h>

#include <libdevcore/Assertions.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace dev
{
namespace test
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

std::string IPCEnvOrDefaultPath()
{
	if (auto path = getenv("ETH_TEST_IPC"))
		return path;

	if (auto home = getenv("HOME"))
		return std::string(home) + "/.ethereum/geth.ipc";

	return std::string{};
}

CommonOptions::CommonOptions(std::string _caption):
	options(_caption,
		po::options_description::m_default_line_length,
		po::options_description::m_default_line_length - 23
	)
{
	options.add_options()
		("evm-version", po::value(&evmVersionString), "which evm version to use")
		("testpath", po::value<fs::path>(&this->testPath)->default_value(dev::test::testPath()), "path to test files")
		("ipcpath", po::value<fs::path>(&ipcPath)->default_value(IPCEnvOrDefaultPath()), "path to ipc socket")
		("no-ipc", po::bool_switch(&disableIPC), "disable semantic tests")
		("no-smt", po::bool_switch(&disableSMT), "disable SMT checker");
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

//	if (!disableIPC)
//	{
//		assertThrow(
//			!ipcPath.empty(),
//			ConfigException,
//			"No ipc path specified. The --ipcpath argument must not be empty when given."
//		);
//		assertThrow(
//			fs::exists(ipcPath),
//			ConfigException,
//			"Invalid ipc path specified."
//		);
//	}
}

bool CommonOptions::parse(int argc, char const* const* argv)
{
	po::variables_map arguments;

	po::command_line_parser cmdLineParser(argc, argv);
	cmdLineParser.options(options);
	po::store(cmdLineParser.run(), arguments);
	po::notify(arguments);

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

}

}
