
#include <string>
#include <iostream>

#include <libdevcore/Common.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/CommonIO.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/ASTPrinter.h>
#include <libsolidity/NameAndTypeResolver.h>

namespace dev {
namespace solidity {

ptr<ContractDefinition> parseAST(std::string const& _source)
{
	ptr<Scanner> scanner = std::make_shared<Scanner>(CharStream(_source));
	Parser parser;
	return parser.parse(scanner);
}

} } // end namespaces

void help()
{
	std::cout
		<< "Usage solc [OPTIONS] <file>" << std::endl
		<< "Options:" << std::endl
		<< "    -h,--help  Show this help message and exit." << std::endl
		<< "    -V,--version  Show the version and exit." << std::endl;
	exit(0);
}

void version()
{
	std::cout
			<< "solc, the solidity complier commandline interface " << dev::Version << std::endl
			<< "  by Christian <c@ethdev.com>, (c) 2014." << std::endl
			<< "Build: " << DEV_QUOTED(ETH_BUILD_PLATFORM) << "/" << DEV_QUOTED(ETH_BUILD_TYPE) << std::endl;
	exit(0);
}

int main(int argc, char** argv)
{
	std::string infile;

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg == "-h" || arg == "--help")
			help();
		else if (arg == "-V" || arg == "--version")
			version();
		else
			infile = argv[i];
	}

	std::string src;
	if (infile.empty())
	{
		std::string s;
		while (!std::cin.eof())
		{
			getline(std::cin, s);
			src.append(s);
		}
	} else {
		src = dev::asString(dev::contents(infile));
	}

	std::cout << "Parsing..." << std::endl;
	// @todo catch exception
	dev::solidity::ptr<dev::solidity::ContractDefinition> ast = dev::solidity::parseAST(src);
	std::cout << "Syntax tree for the contract:" << std::endl;
	dev::solidity::ASTPrinter printer(ast, src);
	printer.print(std::cout);
	std::cout << "Resolving identifiers..." << std::endl;
	dev::solidity::NameAndTypeResolver resolver;
	resolver.resolveNamesAndTypes(*ast.get());
	return 0;
}
