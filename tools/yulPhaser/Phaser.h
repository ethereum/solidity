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
/**
 * Contains the main class that controls yul-phaser based on command-line parameters.
 */

#pragma once

#include <boost/program_options.hpp>

#include <istream>
#include <ostream>
#include <string>

namespace solidity::phaser
{

enum class Algorithm
{
	Random,
	GEWEP,
};

std::istream& operator>>(std::istream& _inputStream, solidity::phaser::Algorithm& _algorithm);
std::ostream& operator<<(std::ostream& _outputStream, solidity::phaser::Algorithm _algorithm);

/**
 * Main class that controls yul-phaser based on command-line parameters. The class is responsible
 * for command-line parsing, initialisation of global objects (like the random number generator),
 * creating instances of main components and running the genetic algorithm.
 */
class Phaser
{
public:
	static int main(int argc, char** argv);

private:
	struct CommandLineParsingResult
	{
		int exitCode;
		boost::program_options::variables_map arguments;
	};

	static CommandLineParsingResult parseCommandLine(int _argc, char** _argv);
	static void initialiseRNG(boost::program_options::variables_map const& _arguments);

	static void runAlgorithm(std::string const& _sourcePath, Algorithm _algorithm);
};

}
