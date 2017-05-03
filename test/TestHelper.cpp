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
/** @file TestHelper.h
* @author Marko Simovic <markobarko@gmail.com>
* @date 2014
*/

#include <boost/test/framework.hpp>
#include "TestHelper.h"
using namespace std;
using namespace dev::test;

Options const& Options::get()
{
	static Options instance;
	return instance;
}

Options::Options()
{
	auto const& suite = boost::unit_test::framework::master_test_suite();
	for (auto i = 0; i < suite.argc; i++)
		if (string(suite.argv[i]) == "--ipcpath" && i + 1 < suite.argc)
		{
			ipcPath = suite.argv[i + 1];
			i++;
		}
		else if (string(suite.argv[i]) == "--optimize")
			optimize = true;
		else if (string(suite.argv[i]) == "--show-messages")
			showMessages = true;
		else if (string(suite.argv[i]) == "--no-ipc")
			disableIPC = true;

	if (!disableIPC && ipcPath.empty())
		if (auto path = getenv("ETH_TEST_IPC"))
			ipcPath = path;
}
