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

#include <libyul/optimiser/FunctionCallFinder.h>
#include <libyul/AST.h>

using namespace solidity;
using namespace solidity::yul;

namespace
{
template<typename Base, typename ResultType>
class FunctionCallFinderBase: Base
{
public:
	using MaybeConstBlock = std::conditional_t<std::is_const_v<ResultType>, Block const, Block>;
	static std::vector<ResultType*> run(MaybeConstBlock& _block, YulName _functionName)
	{
		FunctionCallFinderBase functionCallFinder(_functionName);
		functionCallFinder(_block);
		return functionCallFinder.m_calls;
	}
private:
	explicit FunctionCallFinderBase(YulName _functionName): m_functionName(_functionName) {}
	using Base::operator();
	void operator()(ResultType& _functionCall) override
	{
		Base::operator()(_functionCall);
		if (_functionCall.functionName.name == m_functionName)
			m_calls.emplace_back(&_functionCall);
	}
	YulName m_functionName;
	std::vector<ResultType*> m_calls;
};
}

std::vector<FunctionCall*> FunctionCallFinder::run(Block& _block, YulName _functionName)
{
	return FunctionCallFinderBase<ASTModifier, FunctionCall>::run(_block, _functionName);
}

std::vector<FunctionCall const*> FunctionCallFinder::run(Block const& _block, YulName _functionName)
{
	return FunctionCallFinderBase<ASTWalker, FunctionCall const>::run(_block, _functionName);
}
