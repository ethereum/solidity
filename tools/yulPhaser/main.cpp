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

#include <tools/yulPhaser/Exceptions.h>
#include <tools/yulPhaser/Phaser.h>

#include <libsolutil/Exceptions.h>

#include <iostream>

int main(int argc, char** argv)
{
	try
	{
		solidity::phaser::Phaser::main(argc, argv);
		return 0;
	}
	catch (boost::program_options::error const& exception)
	{
		// Bad input data. Invalid command-line parameters.

		std::cerr << std::endl;
		std::cerr << "ERROR: " << exception.what() << std::endl;
		return 2;
	}
	catch (solidity::phaser::BadInput const& exception)
	{
		// Bad input data. Syntax errors in the input program, semantic errors in command-line
		// parameters, etc.

		std::cerr << std::endl;
		std::cerr << "ERROR: " << exception.what() << std::endl;
		return 2;
	}
	catch (...)
	{
		std::cerr << "Uncaught exception:" << std::endl;
		std::cerr << boost::current_exception_diagnostic_information() << std::endl;
		return 2;
	}
}
