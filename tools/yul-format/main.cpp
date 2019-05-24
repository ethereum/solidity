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

boost::optional<std::string> prettyPrint(
	std::string const& _sourceCode,
	std::string const& _sourceName,
	yul::EVMDialect const& _evmDialect,
	langutil::ErrorReporter& _errorReporter
)
{
	langutil::EVMVersion const evmVersion = *langutil::EVMVersion::fromString("petersburg");
	yul::EVMDialect const& evmDialect = yul::EVMDialect::strictAssemblyForEVM(evmVersion);
	yul::Parser parser{_errorReporter, evmDialect};
	langutil::CharStream source{_sourceCode, _sourceName};
	auto scanner = make_shared<langutil::Scanner>(source);
	shared_ptr<yul::Block> ast = parser.parse(scanner, true);

	if (_errorReporter.hasErrors())
		return boost::none;
	else
		return {yul::AsmPrinter{true}(*ast)};
}

struct Flags
{
	langutil::EVMVersion evmVersion;
	std::string sourceName;
	std::string sourceCode;
};

boost::optional<Flags> parseArgs(int argc, const char* argv[])
{
	po::options_description options(
		R"(yul-format, the Yul source code pretty printer.
Usage: yul-format [Options] < input
Reads a single source from stdin and prints it with proper formatting.

Allowed options)",
		po::options_description::m_default_line_length,
		po::options_description::m_default_line_length - 23);

	options.add_options()
		("help", "Show this help screen.")
		(
			"evm-version",
			po::value<string>()->value_name("version"),
			"Select desired EVM version. Either homestead, tangerineWhistle, spuriousDragon, byzantium, constantinople or petersburg (default)."
		)
		("input-file", po::value<string>(), "Input file to format.");

	po::positional_options_description filesPositions;
	filesPositions.add("input-file", -1);

	po::variables_map arguments;
	try
	{
		po::command_line_parser cmdLineParser(argc, argv);
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

	langutil::EVMVersion evmVersion = *langutil::EVMVersion::fromString("petersburg");
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
	if (arguments.count("input-file"))
		return Flags{
			evmVersion,
			arguments["input-file"].as<string>(),
			dev::readFileAsString(arguments["input-file"].as<string>())
		};
	else
		return Flags{
			evmVersion,
			"stdin",
			dev::readStandardInput()
		};
}

int main(int argc, const char* argv[])
{
	boost::optional<Flags> flags = parseArgs(argc, argv);
	if (!flags)
		return EXIT_FAILURE;

	langutil::ErrorList errors;
	langutil::ErrorReporter errorReporter{errors};

	optional<string> const pretty = prettyPrint(
		flags->sourceCode,
		flags->sourceName,
		yul::EVMDialect::strictAssemblyForEVM(flags->evmVersion),
		errorReporter
	);

	if (!errorReporter.hasErrors())
		cout << *pretty;
	else
		for (shared_ptr<langutil::Error const> const& error : errors)
			langutil::SourceReferenceFormatterHuman{cerr, true}.printErrorInformation(*error);

	return EXIT_SUCCESS;
}
