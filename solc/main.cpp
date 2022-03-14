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

using namespace std;
using namespace solidity;


int main(int argc, char** argv)
{
	try
	{
		solidity::frontend::CommandLineInterface cli(cin, cout, cerr);
		return cli.run(argc, argv) ? 0 : 1;
	}
	catch (smtutil::SMTLogicError const& _exception)
	{
		cerr << "SMT logic error:" << endl;
		cerr << boost::diagnostic_information(_exception);
		return 1;
	}
	catch (langutil::UnimplementedFeatureError const& _exception)
	{
		cerr << "Unimplemented feature:" << endl;
		cerr << boost::diagnostic_information(_exception);
		return 1;
	}
	catch (langutil::InternalCompilerError const& _exception)
	{
		cerr << "Internal compiler error:" << endl;
		cerr << boost::diagnostic_information(_exception);
		return 1;
	}
	catch (boost::exception const& _exception)
	{
		cerr << "Uncaught exception:" << endl;
		cerr << boost::diagnostic_information(_exception) << endl;
		return 1;
	}
	catch (std::exception const& _exception)
	{
		cerr << "Uncaught exception:" << endl;
		cerr << boost::diagnostic_information(_exception) << endl;
		return 1;
	}
	catch (...)
	{
		cerr << "Uncaught exception" << endl;
		cerr << boost::current_exception_diagnostic_information() << endl;
		return 1;
	}
}
