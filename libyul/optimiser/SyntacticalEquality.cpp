/*(
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

#include <libyul/optimiser/SyntacticalEquality.h>

#include <libyul/AST.h>
#include <libyul/Utilities.h>
#include <libyul/Exceptions.h>

#include <libsolutil/CommonData.h>

using namespace solidity;
using namespace solidity::yul;

bool SyntacticallyEqual::operator()(Expression const& _lhs, Expression const& _rhs)
{
	return std::visit([this](auto&& _lhsExpr, auto&& _rhsExpr) -> bool {
		// ``this->`` is redundant, but required to work around a bug present in gcc 6.x.
		return this->expressionEqual(_lhsExpr, _rhsExpr);
	}, _lhs, _rhs);
}

bool SyntacticallyEqual::operator()(Statement const& _lhs, Statement const& _rhs)
{
	return std::visit([this](auto&& _lhsStmt, auto&& _rhsStmt) -> bool {
		// ``this->`` is redundant, but required to work around a bug present in gcc 6.x.
		return this->statementEqual(_lhsStmt, _rhsStmt);
	}, _lhs, _rhs);
}

bool SyntacticallyEqual::expressionEqual(FunctionCall const& _lhs, FunctionCall const& _rhs)
{
	return
		expressionEqual(_lhs.functionName, _rhs.functionName) &&
		util::containerEqual(_lhs.arguments, _rhs.arguments, [this](Expression const& _lhsExpr, Expression const& _rhsExpr) -> bool {
			return (*this)(_lhsExpr, _rhsExpr);
		});
}

bool SyntacticallyEqual::expressionEqual(Identifier const& _lhs, Identifier const& _rhs)
{
	auto lhsIt = m_identifiersLHS.find(_lhs.name);
	auto rhsIt = m_identifiersRHS.find(_rhs.name);
	return
		(lhsIt == m_identifiersLHS.end() && rhsIt == m_identifiersRHS.end() && _lhs.name == _rhs.name) ||
		(lhsIt != m_identifiersLHS.end() && rhsIt != m_identifiersRHS.end() && lhsIt->second == rhsIt->second);
}
bool SyntacticallyEqual::expressionEqual(Literal const& _lhs, Literal const& _rhs)
{
	yulAssert(validLiteral(_lhs), "Invalid lhs literal during syntactical equality check");
	yulAssert(validLiteral(_rhs), "Invalid rhs literal during syntactical equality check");

	return _lhs.value == _rhs.value;
}

bool SyntacticallyEqual::statementEqual(ExpressionStatement const& _lhs, ExpressionStatement const& _rhs)
{
	return (*this)(_lhs.expression, _rhs.expression);
}
bool SyntacticallyEqual::statementEqual(Assignment const& _lhs, Assignment const& _rhs)
{
	return util::containerEqual(
		_lhs.variableNames,
		_rhs.variableNames,
		[this](Identifier const& _lhsVarName, Identifier const& _rhsVarName) -> bool {
			return this->expressionEqual(_lhsVarName, _rhsVarName);
		}
	) && (*this)(*_lhs.value, *_rhs.value);
}

bool SyntacticallyEqual::statementEqual(VariableDeclaration const& _lhs, VariableDeclaration const& _rhs)
{
	// first visit expression, then variable declarations
	if (!compareUniquePtr<Expression, &SyntacticallyEqual::operator()>(_lhs.value, _rhs.value))
		return false;
	return util::containerEqual(_lhs.variables, _rhs.variables, [this](NameWithDebugData const& _lhsVarName, NameWithDebugData const& _rhsVarName) -> bool {
		return this->visitDeclaration(_lhsVarName, _rhsVarName);
	});
}

bool SyntacticallyEqual::statementEqual(FunctionDefinition const& _lhs, FunctionDefinition const& _rhs)
{
	auto compare = [this](NameWithDebugData const& _lhsVarName, NameWithDebugData const& _rhsVarName) -> bool {
		return this->visitDeclaration(_lhsVarName, _rhsVarName);
	};
	// first visit parameter declarations, then body
	if (!util::containerEqual(_lhs.parameters, _rhs.parameters, compare))
		return false;
	if (!util::containerEqual(_lhs.returnVariables, _rhs.returnVariables, compare))
		return false;
	return statementEqual(_lhs.body, _rhs.body);
}

bool SyntacticallyEqual::statementEqual(If const& _lhs, If const& _rhs)
{
	return
		compareUniquePtr<Expression, &SyntacticallyEqual::operator()>(_lhs.condition, _rhs.condition) &&
		statementEqual(_lhs.body, _rhs.body);
}

bool SyntacticallyEqual::statementEqual(Switch const& _lhs, Switch const& _rhs)
{
	std::set<Case const*, SwitchCaseCompareByLiteralValue> lhsCases;
	std::set<Case const*, SwitchCaseCompareByLiteralValue> rhsCases;
	for (auto const& lhsCase: _lhs.cases)
		lhsCases.insert(&lhsCase);
	for (auto const& rhsCase: _rhs.cases)
		rhsCases.insert(&rhsCase);
	return
		compareUniquePtr<Expression, &SyntacticallyEqual::operator()>(_lhs.expression, _rhs.expression) &&
		util::containerEqual(lhsCases, rhsCases, [this](Case const* _lhsCase, Case const* _rhsCase) -> bool {
			return this->switchCaseEqual(*_lhsCase, *_rhsCase);
		});
}


bool SyntacticallyEqual::switchCaseEqual(Case const& _lhs, Case const& _rhs)
{
	return
		compareUniquePtr<Literal, &SyntacticallyEqual::expressionEqual>(_lhs.value, _rhs.value) &&
		statementEqual(_lhs.body, _rhs.body);
}

bool SyntacticallyEqual::statementEqual(ForLoop const& _lhs, ForLoop const& _rhs)
{
	return
		statementEqual(_lhs.pre, _rhs.pre) &&
		compareUniquePtr<Expression, &SyntacticallyEqual::operator()>(_lhs.condition, _rhs.condition) &&
		statementEqual(_lhs.body, _rhs.body) &&
		statementEqual(_lhs.post, _rhs.post);
}

bool SyntacticallyEqual::statementEqual(Block const& _lhs, Block const& _rhs)
{
	return util::containerEqual(_lhs.statements, _rhs.statements, [this](Statement const& _lhsStmt, Statement const& _rhsStmt) -> bool {
		return (*this)(_lhsStmt, _rhsStmt);
	});
}

bool SyntacticallyEqual::visitDeclaration(NameWithDebugData const& _lhs, NameWithDebugData const& _rhs)
{
	std::size_t id = m_idsUsed++;
	m_identifiersLHS[_lhs.name] = id;
	m_identifiersRHS[_rhs.name] = id;
	return true;
}

bool SyntacticallyEqualExpression::operator()(Expression const& _lhs, Expression const& _rhs) const
{
	return SyntacticallyEqual{}(_lhs, _rhs);
}

