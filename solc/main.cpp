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

#include <clocale>
#include <iostream>

using namespace std;
using namespace solidity;

/*
The equivalent of setlocale(LC_ALL, "C") is called before any user code is run.
If the user has an invalid environment setting then it is possible for the call
to set locale to fail, so there are only two possible actions, the first is to
throw a runtime exception and cause the program to quit (default behaviour),
or the second is to modify the environment to something sensible (least
surprising behaviour).

The follow code produces the least surprising behaviour. It will use the user
specified default locale if it is valid, and if not then it will modify the
environment the process is running in to use a sensible default. This also means
that users do not need to install language packs for their OS.
*/
static void setDefaultOrCLocale()
{
#if __unix__
	if (!std::setlocale(LC_ALL, ""))
	{
		setenv("LC_ALL", "C", 1);
	}
#endif
}

int main(int argc, char** argv)
{
	try
	{
		setDefaultOrCLocale();
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
