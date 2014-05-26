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
#include "BuildInfo.h"
using namespace std;
using namespace eth;

void help()
{
	cout
		<< "Usage lllc [OPTIONS] <file>" << endl
        << "Options:" << endl
        << "    -h,--help  Show this help message and exit." << endl
        << "    -V,--version  Show the version and exit." << endl;
        exit(0);
}

void version()
{
	cout << "LLLC, the Lovely Little Language Compiler " << ETH_QUOTED(ETH_VERSION) << endl;
	cout << "  By Gav Wood, (c) 2014." << endl;
	cout << "Build: " << ETH_QUOTED(ETH_BUILD_PLATFORM) << "/" << ETH_QUOTED(ETH_BUILD_TYPE) << endl;
	exit(0);
}

enum Mode { Binary, Hex, ParseTree };

int main(int argc, char** argv)
{
	string infile;
	Mode mode = Hex;

	for (int i = 1; i < argc; ++i)
	{
		string arg = argv[i];
		if (arg == "-h" || arg == "--help")
			help();
		else if (arg == "-b" || arg == "--binary")
			mode = Binary;
		else if (arg == "-h" || arg == "--hex")
			mode = Hex;
		else if (arg == "-t" || arg == "--parse-tree")
			mode = ParseTree;
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

	if (src.empty())
		cerr << "Empty file." << endl;
	else if (mode == Binary || mode == Hex)
	{
		vector<string> errors;
		auto bs = compileLLL(src, &errors);
		if (mode == Hex)
			cout << toHex(bs) << endl;
		else if (mode == Binary)
			cout.write((char const*)bs.data(), bs.size());
		for (auto const& i: errors)
			cerr << i << endl;
	}
	else if (mode == ParseTree)
	{
		cout << parseLLL(src) << endl;
	}

	return 0;
}
