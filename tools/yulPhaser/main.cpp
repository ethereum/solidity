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
		return 1;
	}
	catch (solidity::phaser::BadInput const& exception)
	{
		// Bad input data. Syntax errors in the input program, semantic errors in command-line
		// parameters, etc.

		std::cerr << std::endl;
		std::cerr << "ERROR: " << exception.what() << std::endl;
		return 1;
	}
	catch (solidity::util::Exception const& exception)
	{
		// Something's seriously wrong. Probably a bug in the program or a missing handler (which
		// is really also a bug). The exception should have been handled gracefully by this point
		// if it's something that can happen in normal usage. E.g. an error in the input or a
		// failure of some part of the system that's outside of control of the application (disk,
		// network, etc.). The bug should be reported and investigated so our job here is just to
		// provide as much useful information about it as possible.

		std::cerr << std::endl;
		std::cerr << "UNCAUGHT EXCEPTION!" << std::endl;

		// We can print some useful diagnostic info for this particular exception type.
		std::cerr << "Location: " << exception.lineInfo() << std::endl;

		char const* const* function = boost::get_error_info<boost::throw_function>(exception);
		if (function != nullptr)
			std::cerr << "Function: " << *function << std::endl;

		// Let it crash. The terminate() will print some more stuff useful for debugging like
		// what() and the actual exception type.
		throw;
	}
	catch (std::exception const&)
	{
		// Again, probably a bug but this time it's just plain std::exception so there's no point
		// in doing anything special. terminate() will do an adequate job.
		std::cerr << std::endl;
		std::cerr << "UNCAUGHT EXCEPTION!" << std::endl;
		throw;
	}
	catch (...)
	{
		// Some people don't believe these exist.
		// I have no idea what this is and it's flying towards me so technically speaking it's an
		// unidentified flying object.
		std::cerr << std::endl;
		std::cerr << "UFO SPOTTED!" << std::endl;
		throw;
	}
}
