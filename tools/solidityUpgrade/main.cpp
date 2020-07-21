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
#include <tools/solidityUpgrade/SourceUpgrade.h>

#include <libsolutil/CommonIO.h>
#include <libsolutil/AnsiColorized.h>

//#include <test/Common.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <queue>
#include <regex>

#if defined(_WIN32)
#include <windows.h>
#endif

using namespace solidity;
using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace
{

void setupTerminal()
{
#if defined(_WIN32) && defined(ENABLE_VIRTUAL_TERMINAL_PROCESSING)
	// Set output mode to handle virtual terminal (ANSI escape sequences)
	// ignore any error, as this is just a "nice-to-have"
	// only windows needs to be taken care of, as other platforms (Linux/OSX) support them natively.
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		return;

	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode))
		return;

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(hOut, dwMode))
		return;
#endif
}

}

int main(int argc, char** argv)
{
	setupTerminal();

	tools::SourceUpgrade upgrade;
	if (!upgrade.parseArguments(argc, argv))
		return 1;
	upgrade.printPrologue();

	try
	{
		if (!upgrade.processInput())
			return 1;
	}
	catch (boost::exception const& _exception)
	{
		cerr << "Exception while processing input: " << boost::diagnostic_information(_exception) << endl;
	}

	return 0;
}
