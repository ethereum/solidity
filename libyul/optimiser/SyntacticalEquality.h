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
 * Component that can compare ASTs for equality on a syntactic basis.
 */

#pragma once

#include <libyul/AsmDataForward.h>
#include <libyul/YulString.h>

#include <map>
#include <type_traits>

namespace yul
{


/**
 * Component that can compare ASTs for equality on a syntactic basis.
 * Ignores source locations and allows for different variable names but requires exact matches otherwise.
 *
 * Prerequisite: Disambiguator (unless only expressions are compared)
 */
class SyntacticallyEqual
{
public:
	bool operator()(Expression const& _lhs, Expression const& _rhs);
	bool operator()(Statement const& _lhs, Statement const& _rhs);

	bool expressionEqual(FunctionalInstruction const& _lhs, FunctionalInstruction const& _rhs);
	bool expressionEqual(FunctionCall const& _lhs, FunctionCall const& _rhs);
	bool expressionEqual(Identifier const& _lhs, Identifier const& _rhs);
	bool expressionEqual(Literal const& _lhs, Literal const& _rhs);

	bool statementEqual(ExpressionStatement const& _lhs, ExpressionStatement const& _rhs);
	bool statementEqual(Assignment const& _lhs, Assignment const& _rhs);
	bool statementEqual(VariableDeclaration const& _lhs, VariableDeclaration const& _rhs);
	bool statementEqual(FunctionDefinition const& _lhs, FunctionDefinition const& _rhs);
	bool statementEqual(If const& _lhs, If const& _rhs);
	bool statementEqual(Switch const& _lhs, Switch const& _rhs);
	bool switchCaseEqual(Case const& _lhs, Case const& _rhs);
	bool statementEqual(ForLoop const& _lhs, ForLoop const& _rhs);
	bool statementEqual(Break const&, Break const&) { return true; }
	bool statementEqual(Continue const&, Continue const&) { return true; }
	bool statementEqual(Leave const&, Leave const&) { return true; }
	bool statementEqual(Block const& _lhs, Block const& _rhs);
private:
	bool statementEqual(Instruction const& _lhs, Instruction const& _rhs);

	bool visitDeclaration(TypedName const& _lhs, TypedName const& _rhs);

	template<typename U, typename V>
	bool expressionEqual(U const&, V const&, std::enable_if_t<!std::is_same<U, V>::value>* = nullptr)
	{
		return false;
	}

	template<typename U, typename V>
	bool statementEqual(U const&, V const&, std::enable_if_t<!std::is_same<U, V>::value>* = nullptr)
	{
		return false;
	}

	template<typename T, bool (SyntacticallyEqual::*CompareMember)(T const&, T const&)>
	bool compareUniquePtr(std::unique_ptr<T> const& _lhs, std::unique_ptr<T> const& _rhs)
	{
		return (_lhs == _rhs) || (_lhs && _rhs && (this->*CompareMember)(*_lhs, *_rhs));
	}

	std::size_t m_idsUsed = 0;
	std::map<YulString, std::size_t> m_identifiersLHS;
	std::map<YulString, std::size_t> m_identifiersRHS;
};

}
