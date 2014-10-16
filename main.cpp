
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

void printSourcePart(std::ostream& _stream, Location const& _location, Scanner const& _scanner)
{
	int startLine;
	int startColumn;
	std::tie(startLine, startColumn) = _scanner.translatePositionToLineColumn(_location.start);
	_stream << " starting at line " << (startLine + 1) << ", column " << (startColumn + 1) << "\n";
	int endLine;
	int endColumn;
	std::tie(endLine, endColumn) = _scanner.translatePositionToLineColumn(_location.end);
	if (startLine == endLine)
	{
		_stream << _scanner.getLineAtPosition(_location.start) << "\n"
				<< std::string(startColumn, ' ') << "^";
		if (endColumn > startColumn + 2)
			_stream << std::string(endColumn - startColumn - 2, '-');
		if (endColumn > startColumn + 1)
			_stream << "^";
		_stream << "\n";
	}
	else
	{
		_stream << _scanner.getLineAtPosition(_location.start) << "\n"
				<< std::string(startColumn, ' ') << "^\n"
				<< "Spanning multiple lines.\n";
	}
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
	catch (ParserError const& exc)
	{
		int line;
		int column;
		std::tie(line, column) = scanner->translatePositionToLineColumn(exc.getPosition());
		std::cerr << exc.what() << " at line " << (line + 1) << ", column " << (column + 1) << std::endl;
		std::cerr << scanner->getLineAtPosition(exc.getPosition()) << std::endl;
		std::cerr << std::string(column, ' ') << "^" << std::endl;
		return -1;
	}

	dev::solidity::NameAndTypeResolver resolver;
	try
	{
		resolver.resolveNamesAndTypes(*ast.get());
	}
	catch (DeclarationError const& exc)
	{
		std::cerr << exc.what() << std::endl;
		printSourcePart(std::cerr, exc.getLocation(), *scanner);
		return -1;
	}
	catch (TypeError const& exc)
	{
		std::cerr << exc.what() << std::endl;
		printSourcePart(std::cerr, exc.getLocation(), *scanner);
		return -1;
	}

	std::cout << "Syntax tree for the contract:" << std::endl;
	dev::solidity::ASTPrinter printer(ast, sourceCode);
	printer.print(std::cout);
	return 0;
}
