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

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/AST.h>
#include <libyul/Dialect.h>
#include <libyul/Utilities.h>

using namespace solidity;
using namespace solidity::yul;

namespace
{
template<typename Base, typename ResultType>
class MaybeConstFunctionCallFinder: Base
{
public:
	using MaybeConstBlock = std::conditional_t<std::is_const_v<ResultType>, Block const, Block>;
	static std::vector<ResultType*> run(MaybeConstBlock& _block, std::string_view const _functionName, Dialect const& _dialect)
	{
		MaybeConstFunctionCallFinder functionCallFinder(_functionName, _dialect);
		functionCallFinder(_block);
		return functionCallFinder.m_calls;
	}
private:
	explicit MaybeConstFunctionCallFinder(std::string_view const _functionName, Dialect const& _dialect):
		m_dialect(_dialect), m_functionName(_functionName), m_calls() {}

	using Base::operator();
	void operator()(ResultType& _functionCall) override
	{
		Base::operator()(_functionCall);
		if (resolveFunctionName(_functionCall.functionName, m_dialect) == m_functionName)
			m_calls.emplace_back(&_functionCall);
	}
	Dialect const& m_dialect;
	std::string_view m_functionName;
	std::vector<ResultType*> m_calls;
};
}

std::vector<FunctionCall*> solidity::yul::findFunctionCalls(Block& _block, std::string_view const _functionName, Dialect const& _dialect)
{
	return MaybeConstFunctionCallFinder<ASTModifier, FunctionCall>::run(_block, _functionName, _dialect);
}

std::vector<FunctionCall const*> solidity::yul::findFunctionCalls(Block const& _block, std::string_view const _functionName, Dialect const& _dialect)
{
	return MaybeConstFunctionCallFinder<ASTWalker, FunctionCall const>::run(_block, _functionName, _dialect);
}
