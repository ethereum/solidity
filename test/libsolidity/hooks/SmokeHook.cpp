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

#include <test/libsolidity/hooks/SmokeHook.h>

#include <test/libsolidity/util/TestFunctionCall.h>

#include <iostream>

namespace solidity::frontend::test
{

void SmokeHook::beginTestCase() { m_count = 0; }

std::vector<std::string> SmokeHook::afterFunctionCall(TestFunctionCall const& _call)
{
	std::vector<std::string> result;
	if (_call.call().kind == FunctionCall::Kind::Builtin && _call.call().signature == "smoke.reaction")
	{
		for (int i = 0; i <= m_count; ++i)
		{
			std::stringstream stream;
			stream << i + 1 << " / " << m_count + 1;
			result.emplace_back(stream.str());
		}
		++m_count;
	}
	return result;
}

} // namespace  solidity::frontend::test
