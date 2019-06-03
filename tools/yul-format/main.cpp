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

#include <libyul/AsmParser.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AssemblyStack.h>
#include <libyul/Dialect.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/SourceReferenceFormatterHuman.h>

#include <libdevcore/CommonIO.h>

#include <cstdlib>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

using boost::optional;
using namespace std;

namespace po = boost::program_options;

struct Flags
{
	langutil::EVMVersion evmVersion;
	yul::AssemblyStack::Language language;
	std::string sourceName;
	std::string sourceCode;
};

static boost::optional<yul::AssemblyStack::Language> parseAssemblyLanguageId(string const& name)
{
	using Language = yul::AssemblyStack::Language;

	if (name == "yul")
		return {Language::Yul};
	else if (name == "assembly")
		return {Language::Assembly};
	else if (name == "strict-assembly")
		return {Language::StrictAssembly};
	else if (name == "ewasm")
		return {Language::EWasm};

	return boost::none;
}

boost::optional<Flags> parseArgs(int _argc, char const* _argv[])
{
	po::options_description options(
		R"(yul-format, the Yul source code pretty printer.
Usage: yul-format [Options] < input
Reads a single source from stdin and prints it with proper formatting.

Allowed options)",
		po::options_description::m_default_line_length,
		po::options_description::m_default_line_length - 23);

	options.add_options()
		("help,h", "Show this help screen.")
		(
			"evm-version",
			po::value<string>()->value_name("version"),
			"Select desired EVM version. Either homestead, tangerineWhistle, spuriousDragon, byzantium, constantinople or petersburg (default)."
		)
		("lang", po::value<string>(), "Language to format. One of yul, assembly, strict-assembly, ewasm.")
		("input-file", po::value<string>(), "Input file to format.");

	po::positional_options_description filesPositions;
	filesPositions.add("input-file", -1);

	po::variables_map arguments;
	try
	{
		po::command_line_parser cmdLineParser(_argc, _argv);
		cmdLineParser.options(options).positional(filesPositions);
		po::store(cmdLineParser.run(), arguments);
	}
	catch (po::error const& _exception)
	{
		cerr << _exception.what() << endl;
		return boost::none;
	}

	if (arguments.count("help"))
	{
		cout << options;
		return boost::none;
	}

	langutil::EVMVersion evmVersion = langutil::EVMVersion::petersburg();
	if (arguments.count("evm-version"))
	{
		string versionOptionStr = arguments["evm-version"].as<string>();
		boost::optional<langutil::EVMVersion> value = langutil::EVMVersion::fromString(versionOptionStr);
		if (!value)
		{
			cerr << "Invalid option for --evm-version: " << versionOptionStr << endl;
			return boost::none;
		}
		evmVersion = *value;
	}
	yul::AssemblyStack::Language lang = yul::AssemblyStack::Language::Yul;
	if (arguments.count("lang"))
	{
		auto parsedLangId = parseAssemblyLanguageId(arguments["lang"].as<string>());
		if (!parsedLangId)
		{
			cerr << "Invalid language ID. Try --help.\n";
			return boost::none;
		}
		else
			lang = *parsedLangId;
	}
	tuple<string, string> const input = [&]() -> tuple<string, string>
	{
		if (arguments.count("input-file"))
			return make_tuple(
				arguments["input-file"].as<string>(),
				dev::readFileAsString(arguments["input-file"].as<string>())
			);
		else
			return make_tuple(string{"stdin"}, dev::readStandardInput());
	}();
	return Flags{
		evmVersion,
		lang,
		get<0>(input),
		get<1>(input)
	};
}

int main(int argc, char const* argv[])
{
	boost::optional<Flags> flags = parseArgs(argc, argv);
	if (!flags)
		return EXIT_FAILURE;

	yul::AssemblyStack stack{flags->evmVersion, flags->language, {}};
	if (stack.parse(flags->sourceName, flags->sourceCode))
		cout << yul::AsmPrinter{true}(*stack.parseTree().code);
	else
		for (shared_ptr<langutil::Error const> const& error : stack.errors())
			langutil::SourceReferenceFormatterHuman{cerr, true}.printErrorInformation(*error);

	return EXIT_SUCCESS;
}
