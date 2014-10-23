
#include <string>
#include <iostream>

#include <libdevcore/Common.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/CommonIO.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/ASTPrinter.h>
#include <libsolidity/NameAndTypeResolver.h>
#include <libsolidity/Exceptions.h>
#include <libsolidity/SourceReferenceFormatter.h>

using namespace dev;
using namespace solidity;

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
	std::string sourceCode;
	if (infile.empty())
	{
		std::string s;
		while (!std::cin.eof())
		{
			getline(std::cin, s);
			sourceCode.append(s);
		}
	}
	else
		sourceCode = asString(dev::contents(infile));

	ASTPointer<ContractDefinition> ast;
	std::shared_ptr<Scanner> scanner = std::make_shared<Scanner>(CharStream(sourceCode));
	Parser parser;
	try
	{
		ast = parser.parse(scanner);
	}
	catch (ParserError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(std::cerr, exception, "Parser error", *scanner);
		return -1;
	}

	dev::solidity::NameAndTypeResolver resolver;
	try
	{
		resolver.resolveNamesAndTypes(*ast.get());
	}
	catch (DeclarationError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(std::cerr, exception, "Declaration error", *scanner);
		return -1;
	}
	catch (TypeError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(std::cerr, exception, "Type error", *scanner);
		return -1;
	}

	std::cout << "Syntax tree for the contract:" << std::endl;
	dev::solidity::ASTPrinter printer(ast, sourceCode);
	printer.print(std::cout);
	return 0;
}
