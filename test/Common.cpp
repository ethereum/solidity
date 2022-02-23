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
#include <test/EVMHost.h>

#include <libsolutil/Assertions.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace solidity::test
{

namespace
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

std::optional<fs::path> findInDefaultPath(std::string const& lib_name)
{
	auto const searchPath =
	{
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
			return p;
	}
	return std::nullopt;
}

}

CommonOptions::CommonOptions(std::string _caption):
	options(_caption,
		po::options_description::m_default_line_length,
		po::options_description::m_default_line_length - 23
	)
{

}

void CommonOptions::addOptions()
{
	options.add_options()
		("evm-version", po::value(&evmVersionString), "which evm version to use")
		("testpath", po::value<fs::path>(&this->testPath)->default_value(solidity::test::testPath()), "path to test files")
		("vm", po::value<std::vector<fs::path>>(&vmPaths), "path to evmc library, can be supplied multiple times.")
		("ewasm", po::bool_switch(&ewasm)->default_value(ewasm), "tries to automatically find an ewasm vm and enable ewasm test-execution.")
		("batches", po::value<size_t>(&this->batches)->default_value(1), "set number of batches to split the tests into")
		("selected-batch", po::value<size_t>(&this->selectedBatch)->default_value(0), "zero-based number of batch to execute")
		("no-semantic-tests", po::bool_switch(&disableSemanticTests)->default_value(disableSemanticTests), "disable semantic tests")
		("no-smt", po::bool_switch(&disableSMT)->default_value(disableSMT), "disable SMT checker")
		("optimize", po::bool_switch(&optimize)->default_value(optimize), "enables optimization")
		("enforce-via-yul", po::value<bool>(&enforceViaYul)->default_value(enforceViaYul)->implicit_value(true), "Enforce compiling all tests via yul to see if additional tests can be activated.")
		("enforce-compile-to-ewasm", po::bool_switch(&enforceCompileToEwasm)->default_value(enforceCompileToEwasm), "Enforce compiling all tests to Ewasm to see if additional tests can be activated.")
		("enforce-gas-cost", po::value<bool>(&enforceGasTest)->default_value(enforceGasTest)->implicit_value(true), "Enforce checking gas cost in semantic tests.")
		("enforce-gas-cost-min-value", po::value(&enforceGasTestMinValue)->default_value(enforceGasTestMinValue), "Threshold value to enforce adding gas checks to a test.")
		("abiencoderv1", po::bool_switch(&useABIEncoderV1)->default_value(useABIEncoderV1), "enables abi encoder v1")
		("show-messages", po::bool_switch(&showMessages)->default_value(showMessages), "enables message output")
		("show-metadata", po::bool_switch(&showMetadata)->default_value(showMetadata), "enables metadata output");
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
	assertThrow(
		batches > 0,
		ConfigException,
		"Batches needs to be at least 1."
	);
	assertThrow(
		selectedBatch < batches,
		ConfigException,
		"Selected batch has to be less than number of batches."
	);

	if (enforceGasTest)
	{
		assertThrow(
			evmVersion() == langutil::EVMVersion{},
			ConfigException,
			"Gas costs can only be enforced on latest evm version."
		);
		assertThrow(
			useABIEncoderV1 == false,
			ConfigException,
			"Gas costs can only be enforced on abi encoder v2."
		);
	}
}

bool CommonOptions::parse(int argc, char const* const* argv)
{
	po::variables_map arguments;
	addOptions();

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
			BOOST_THROW_EXCEPTION(std::runtime_error(errorMessage.str()));
		}

	if (vmPaths.empty())
	{
		if (auto envPath = getenv("ETH_EVMONE"))
			vmPaths.emplace_back(envPath);
		else if (auto repoPath = findInDefaultPath(evmoneFilename))
			vmPaths.emplace_back(*repoPath);
		else
			vmPaths.emplace_back(evmoneFilename);
		if (ewasm) {
			if (auto envPath = getenv("ETH_HERA"))
				vmPaths.emplace_back(envPath);
			else if (auto repoPath = findInDefaultPath(heraFilename))
				vmPaths.emplace_back(*repoPath);
			else
				vmPaths.emplace_back(heraFilename);
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
			BOOST_THROW_EXCEPTION(std::runtime_error("Invalid EVM version: " + evmVersionString));
		return *version;
	}
	else
		return langutil::EVMVersion();
}


CommonOptions const& CommonOptions::get()
{
	if (!m_singleton)
		BOOST_THROW_EXCEPTION(std::runtime_error("Options not yet constructed!"));

	return *m_singleton;
}

void CommonOptions::setSingleton(std::unique_ptr<CommonOptions const>&& _instance)
{
	m_singleton = std::move(_instance);
}

std::unique_ptr<CommonOptions const> CommonOptions::m_singleton = nullptr;

bool isValidSemanticTestPath(boost::filesystem::path const& _testPath)
{
	bool insideSemanticTests = false;
	fs::path testPathPrefix;
	for (auto const& element: _testPath)
	{
		testPathPrefix /= element;
		if (boost::ends_with(canonical(testPathPrefix).generic_string(), "/test/libsolidity/semanticTests"))
			insideSemanticTests = true;
		if (insideSemanticTests && boost::starts_with(element.string(), "_"))
			return false;
	}
	return true;
}

bool loadVMs(CommonOptions const& _options)
{
	if (_options.disableSemanticTests && !_options.ewasm)
		return true;

	auto [evmSupported, ewasmSupported] = solidity::test::EVMHost::checkVmPaths(_options.vmPaths);
	if (!_options.disableSemanticTests && !evmSupported)
	{
		std::cerr << "Unable to find " << solidity::test::evmoneFilename;
		std::cerr << ". Please disable semantics tests with --no-semantic-tests or provide a path using --vm <path>." << std::endl;
		std::cerr << "You can download it at" << std::endl;
		std::cerr << solidity::test::evmoneDownloadLink << std::endl;
		return false;
	}
	if (_options.ewasm && !ewasmSupported)
	{
		std::cerr << "Unable to find " << solidity::test::heraFilename;
		std::cerr << ". To be able to enable ewasm tests, please provide the path using --vm <path>." << std::endl;
		std::cerr << "You can download it at" << std::endl;
		std::cerr << solidity::test::heraDownloadLink << std::endl;
		return false;
	}
	return true;
}

}
