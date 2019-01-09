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

#include <libyul/Exceptions.h>
#include <libyul/AsmData.h>
#include <libyul/Utilities.h>

#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;

bool SyntacticallyEqual::operator()(Expression const& _lhs, Expression const& _rhs)
{
	return boost::apply_visitor([this](auto&& _lhsExpr, auto&& _rhsExpr) -> bool {
		// ``this->`` is redundant, but required to work around a bug present in gcc 6.x.
		return this->expressionEqual(_lhsExpr, _rhsExpr);
	}, _lhs, _rhs);
}

bool SyntacticallyEqual::operator()(Statement const& _lhs, Statement const& _rhs)
{
	return boost::apply_visitor([this](auto&& _lhsStmt, auto&& _rhsStmt) -> bool {
		// ``this->`` is redundant, but required to work around a bug present in gcc 6.x.
		return this->statementEqual(_lhsStmt, _rhsStmt);
	}, _lhs, _rhs);
}

bool SyntacticallyEqual::expressionEqual(FunctionalInstruction const& _lhs, FunctionalInstruction const& _rhs)
{
	return
		_lhs.instruction == _rhs.instruction &&
		containerEqual(_lhs.arguments, _rhs.arguments, [this](Expression const& _lhsExpr, Expression const& _rhsExpr) -> bool {
			return (*this)(_lhsExpr, _rhsExpr);
		});
}

bool SyntacticallyEqual::expressionEqual(FunctionCall const& _lhs, FunctionCall const& _rhs)
{
	return
		expressionEqual(_lhs.functionName, _rhs.functionName) &&
		containerEqual(_lhs.arguments, _rhs.arguments, [this](Expression const& _lhsExpr, Expression const& _rhsExpr) -> bool {
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
	if (_lhs.kind != _rhs.kind || _lhs.type != _rhs.type)
		return false;
	if (_lhs.kind == LiteralKind::Number)
		return valueOfNumberLiteral(_lhs) == valueOfNumberLiteral(_rhs);
	else
		return _lhs.value == _rhs.value;
}

bool SyntacticallyEqual::statementEqual(ExpressionStatement const& _lhs, ExpressionStatement const& _rhs)
{
	return (*this)(_lhs.expression, _rhs.expression);
}
bool SyntacticallyEqual::statementEqual(Assignment const& _lhs, Assignment const& _rhs)
{
	return containerEqual(
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
	return containerEqual(_lhs.variables, _rhs.variables, [this](TypedName const& _lhsVarName, TypedName const& _rhsVarName) -> bool {
		return this->visitDeclaration(_lhsVarName, _rhsVarName);
	});
}

bool SyntacticallyEqual::statementEqual(FunctionDefinition const& _lhs, FunctionDefinition const& _rhs)
{
	auto compare = [this](TypedName const& _lhsVarName, TypedName const& _rhsVarName) -> bool {
		return this->visitDeclaration(_lhsVarName, _rhsVarName);
	};
	// first visit parameter declarations, then body
	if (!containerEqual(_lhs.parameters, _rhs.parameters, compare))
		return false;
	if (!containerEqual(_lhs.returnVariables, _rhs.returnVariables, compare))
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
	static auto const sortCasesByValue = [](Case const* _lhsCase, Case const* _rhsCase) -> bool {
		return Less<Literal*>{}(_lhsCase->value.get(), _rhsCase->value.get());
	};
	std::set<Case const*, decltype(sortCasesByValue)> lhsCases(sortCasesByValue);
	std::set<Case const*, decltype(sortCasesByValue)> rhsCases(sortCasesByValue);
	for (auto const& lhsCase: _lhs.cases)
		lhsCases.insert(&lhsCase);
	for (auto const& rhsCase: _rhs.cases)
		rhsCases.insert(&rhsCase);
	return
		compareUniquePtr<Expression, &SyntacticallyEqual::operator()>(_lhs.expression, _rhs.expression) &&
		containerEqual(lhsCases, rhsCases, [this](Case const* _lhsCase, Case const* _rhsCase) -> bool {
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

bool SyntacticallyEqual::statementEqual(Instruction const&, Instruction const&)
{
	assertThrow(false, OptimizerException, "");
}

bool SyntacticallyEqual::statementEqual(Label const&, Label const&)
{
	assertThrow(false, OptimizerException, "");
}

bool SyntacticallyEqual::statementEqual(StackAssignment const&, StackAssignment const&)
{
	assertThrow(false, OptimizerException, "");
}

bool SyntacticallyEqual::statementEqual(Block const& _lhs, Block const& _rhs)
{
	return containerEqual(_lhs.statements, _rhs.statements, [this](Statement const& _lhsStmt, Statement const& _rhsStmt) -> bool {
		return (*this)(_lhsStmt, _rhsStmt);
	});
}

bool SyntacticallyEqual::visitDeclaration(TypedName const& _lhs, TypedName const& _rhs)
{
	if (_lhs.type != _rhs.type)
		return false;
	std::size_t id = m_idsUsed++;
	m_identifiersLHS[_lhs.name] = id;
	m_identifiersRHS[_rhs.name] = id;
	return true;
}
