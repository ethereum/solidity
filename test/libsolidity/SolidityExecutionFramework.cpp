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
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Framework for executing Solidity contracts and testing them against C++ implementation.
 */

#include <cstdlib>
#include <boost/test/framework.hpp>
#include <test/libsolidity/SolidityExecutionFramework.h>

using namespace dev::test;
using namespace dev::solidity;
using namespace dev::solidity::test;

SolidityExecutionFramework::SolidityExecutionFramework() :
	ExecutionFramework()
{
}
