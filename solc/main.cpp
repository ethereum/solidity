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
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity commandline compiler.
 */

#include <solc/CommandLineInterface.h>

#include <liblangutil/Exceptions.h>

#include <boost/exception/all.hpp>

#include <iostream>

using namespace solidity;


int main(int argc, char** argv)
{
	try
	{
		solidity::frontend::CommandLineInterface cli(std::cin, std::cout, std::cerr);
		return cli.run(argc, argv) ? 0 : 1;
	}
	catch (smtutil::SMTLogicError const& _exception)
	{
		std::cerr << "SMT logic error:" << std::endl;
		std::cerr << boost::diagnostic_information(_exception);
		return 2;
	}
	catch (langutil::UnimplementedFeatureError const& _exception)
	{
		std::cerr << "Unimplemented feature:" << std::endl;
		std::cerr << boost::diagnostic_information(_exception);
		return 2;
	}
	catch (langutil::InternalCompilerError const& _exception)
	{
		std::cerr << "Internal compiler error:" << std::endl;
		std::cerr << boost::diagnostic_information(_exception);
		return 2;
	}
	catch (...)
	{
		std::cerr << "Uncaught exception:" << std::endl;
		std::cerr << boost::current_exception_diagnostic_information() << std::endl;
		return 2;
	}
}
