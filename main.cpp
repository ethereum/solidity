/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file main.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * Ethereum client.
 */

#include <fstream>
#include <iostream>
#include <liblll/Compiler.h>
#include <libethsupport/CommonIO.h>
#include <libethsupport/CommonData.h>
#include <libethcore/Instruction.h>
#include "BuildInfo.h"
using namespace std;
using namespace eth;

void help()
{
	cout
		<< "Usage lllc [OPTIONS] <file>" << endl
        << "Options:" << endl
		<< "    -b,--binary  Parse, compile and assemble; output byte code in binary." << endl
		<< "    -x,--hex  Parse, compile and assemble; output byte code in hex." << endl
		<< "    -a,--assembly  Only parse and compile; show assembly." << endl
		<< "    -t,--parse-tree  Only parse; show parse tree." << endl
		<< "    -h,--help  Show this help message and exit." << endl
		<< "    -V,--version  Show the version and exit." << endl;
        exit(0);
}

void version()
{
	cout << "LLLC, the Lovely Little Language Compiler " << eth::EthVersion << endl;
	cout << "  By Gav Wood, (c) 2014." << endl;
	cout << "Build: " << ETH_QUOTED(ETH_BUILD_PLATFORM) << "/" << ETH_QUOTED(ETH_BUILD_TYPE) << endl;
	exit(0);
}

enum Mode { Binary, Hex, Assembly, ParseTree, Disassemble };

int main(int argc, char** argv)
{
	unsigned optimise = 1;
	string infile;
	Mode mode = Hex;

	for (int i = 1; i < argc; ++i)
	{
		string arg = argv[i];
		if (arg == "-h" || arg == "--help")
			help();
		else if (arg == "-b" || arg == "--binary")
			mode = Binary;
		else if (arg == "-x" || arg == "--hex")
			mode = Hex;
		else if (arg == "-a" || arg == "--assembly")
			mode = Assembly;
		else if (arg == "-t" || arg == "--parse-tree")
			mode = ParseTree;
		else if ((arg == "-o" || arg == "--optimise") && argc > i + 1)
			optimise = atoi(argv[++i]);
		else if (arg == "-d" || arg == "--disassemble")
			mode = Disassemble;
		else if (arg == "-V" || arg == "--version")
			version();
		else
			infile = argv[i];
	}

	string src;
	if (infile.empty())
	{
		string s;
		while (!cin.eof())
		{
			getline(cin, s);
			src.append(s);
		}
	}
	else
		src = asString(contents(infile));

	vector<string> errors;
	if (src.empty())
		errors.push_back("Empty file.");
	else if (mode == Disassemble)
	{
		cout << disassemble(fromHex(src)) << endl;
	}
	else if (mode == Binary || mode == Hex)
	{
		auto bs = compileLLL(src, optimise ? true : false, &errors);
		if (mode == Hex)
			cout << toHex(bs) << endl;
		else if (mode == Binary)
			cout.write((char const*)bs.data(), bs.size());
	}
	else if (mode == ParseTree)
		cout << parseLLL(src) << endl;
	else if (mode == Assembly)
		cout << compileLLLToAsm(src, optimise ? true : false, &errors) << endl;
	for (auto const& i: errors)
		cerr << i << endl;
	if ( errors.size() )
		return 1;
	return 0;
}
