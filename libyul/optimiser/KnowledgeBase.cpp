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
 * Class that can answer questions about values of variables and their relations.
 */

#include <libyul/optimiser/KnowledgeBase.h>

#include <libyul/AST.h>
#include <libyul/Utilities.h>
#include <libyul/optimiser/SimplificationRules.h>
#include <libyul/optimiser/DataFlowAnalyzer.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Visitor.h>
#include <libsolutil/StringUtils.h>

#include <libyul/AsmPrinter.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <variant>
#include <functional>
#include <queue>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

namespace
{
struct SumExpression;
SumExpression clean(SumExpression _in);

/**
 * Expression of the form k0 + k1 * x2 + x2 * x2 + ...
 * where the ki are u256 constants and the xi are variables.
 * The constant term is using the empty yul string.
 */
struct SumExpression
{
	static SumExpression variable(YulString _name, u256 _multiplicity = u256(1))
	{
		SumExpression result;
		result.coefficients[_name] = move(_multiplicity);
		return result;
	}
	static SumExpression constant(u256 _value)
	{
		return variable(YulString{}, move(_value));
	}
	optional<u256> isConstant() const
	{
		if (coefficients.empty())
			return u256(0);
		else if (coefficients.size() == 1 && coefficients.begin()->first == YulString{})
			return coefficients.begin()->second;
		else
			return nullopt;
	}
	SumExpression operator+(SumExpression const& _other) const
	{
		SumExpression result = *this;
		for (auto&& [var, value]: _other.coefficients)
			result.coefficients[var] += value;
		return clean(move(result));
	}
	SumExpression operator*(u256 const& _factor) const
	{
		if (!_factor)
			return SumExpression{};
		if (_factor == 1)
			return *this;

		SumExpression result;
		for (auto&& [var, value]: coefficients)
			result.coefficients[var] = value * _factor;
		return result;
	}
	string toString() const
	{
		vector<string> result;
		for (auto&& [var, value]: coefficients)
			result.push_back(value.str() + " " + var.str());
		return util::joinHumanReadable(result, " + ");
	}

	map<YulString, u256> coefficients;
};

SumExpression clean(SumExpression _in)
{
	SumExpression result;
	for (auto&& [var, value]: _in.coefficients)
		if (value)
			result.coefficients[var] = move(value);
	return result;
}

optional<SumExpression> operator+(optional<SumExpression> const& _a, optional<SumExpression> const& _b)
{
	if (!_a || !_b)
		return nullopt;
	return *_a + *_b;
}
optional<SumExpression> operator-(optional<SumExpression> const& _a, optional<SumExpression> const& _b)
{
	if (!_a || !_b)
		return nullopt;
	SumExpression result = *_a;
	for (auto&& [var, value]: _b->coefficients)
		result.coefficients[var] -= value;
	return clean(move(result));
}

class SimpleLinearSolver
{
public:
	static optional<u256> simplify(
		EVMDialect const& _dialect,
		std::function<AssignedValue const*(YulString)> _variableValues,
		Expression const& _expr
	)
	{
		SimpleLinearSolver solver(_dialect, _variableValues);
		return solver.simplify(_expr);
	}

private:
	optional<u256> simplify(Expression const& _expr)
	{
		auto value = toSumExpression(_expr);
		if (!value)
			return nullopt;

		size_t iter = 0;
		while (iter++ < 20)
		{
			if (iter % 10 == 0)
			{

				//cout << "iter: " << iter << " current value: " << value->toString() << endl;
			}
			if (auto v = value->isConstant())
				return *v;
			// TODO this will depend on the sorting order of the variables. This is bad and needs to be fixed.
			for (auto&& [var, value]: value->coefficients)
				if (var != YulString{} && !m_expandedVariables.count(var) && !m_expandedFailedVariables.count(var))
					m_variablesToExpand.push(var);
			if (m_variablesToExpand.empty())
				return nullopt;
			YulString var = m_variablesToExpand.front();
			m_variablesToExpand.pop();
			expandVariable(var, *value);
		}
		if (auto v = value->isConstant())
			return *v;
		else
			return nullopt;
	}

private:
	optional<SumExpression> toSumExpression(Expression const& _expr)
	{
		return std::visit(util::GenericVisitor{
			[&](FunctionCall const& _funCall) -> optional<SumExpression> {
				if (BuiltinFunctionForEVM const* builtin = m_dialect.builtin(_funCall.functionName.name))
				{
					if (builtin->instruction == evmasm::Instruction::ADD)
						return toSumExpression(_funCall.arguments.at(0)) + toSumExpression(_funCall.arguments.at(1));
					else if (builtin->instruction == evmasm::Instruction::SUB)
						return toSumExpression(_funCall.arguments.at(0)) - toSumExpression(_funCall.arguments.at(1));
					else
						return std::nullopt;
					// TODO we could also use multiplication by constants.
				}
				return std::nullopt;
			},
			[&](Identifier const& _identifier) -> optional<SumExpression> {
				/*if (m_expandedVariables.count(_identifier.name))
					return m_expandedVariables.at(_identifier.name);
				else*/
					return SumExpression::variable(_identifier.name);
			},
			[&](Literal const& _literal) -> optional<SumExpression> {
				return SumExpression::constant(valueOfLiteral(_literal));
			}
		}, _expr);
	}

	void expandVariable(YulString _variable, SumExpression& _currentExpression)
	{
		//cout << "Expanding " << _variable.str() << endl;
		if (m_expandedFailedVariables.count(_variable) || m_expandedVariables.count(_variable))
			return;
		if (auto assignedValue = m_variableValues(_variable))
			if (assignedValue->value)
				if (auto newValue = toSumExpression(*assignedValue->value))
				{
					expandInExpression(_currentExpression, _variable, *newValue);

					bool expanded = false;
					do
					{
						expanded = false;
						for (auto&& [var, value]: _currentExpression.coefficients)
							if (m_expandedVariables.count(var))
							{
								expandInExpression(_currentExpression, var, m_expandedVariables[var]);
								expanded = true;
								break;
							}
					}
					while (expanded);

				}
		m_expandedFailedVariables.insert(_variable);
	}

	void expandInExpression(SumExpression& _expr, YulString _variable, SumExpression const& _value)
	{
		if (!_expr.coefficients.count(_variable))
			return;
		u256 coefficient = _expr.coefficients[_variable];
		_expr.coefficients.erase(_variable);
		_expr = _expr + _value * coefficient;
	}

	SimpleLinearSolver(
		EVMDialect const& _dialect,
		std::function<AssignedValue const*(YulString)> _variableValues
	): m_dialect(_dialect), m_variableValues(_variableValues)
	{}

	EVMDialect const& m_dialect;
	std::function<AssignedValue const*(YulString)> m_variableValues;

	/// Queue of variables we can still expand in the future.
	queue<YulString> m_variablesToExpand;
	/// Set of variables we expanded in the past and we should directly expand when we
	/// encounter them when expanding other variables.
	map<YulString, SumExpression> m_expandedVariables;
	/// Set of variables we should not expand because their expansion is not linear.
	set<YulString> m_expandedFailedVariables;
};

}

bool KnowledgeBase::knownToBeDifferent(YulString _a, YulString _b)
{
	// Try to use the simplification rules together with the
	// current values to turn `sub(_a, _b)` into a nonzero constant.
	// If that fails, try `eq(_a, _b)`.

	if (optional<u256> difference = differenceIfKnownConstant(_a, _b))
		return difference != 0;

	// TOOD this is not possible anymore.
	// Expression expr2 = simplify(FunctionCall{{}, {{}, "eq"_yulstring}, util::make_vector<Expression>(Identifier{{}, _a}, Identifier{{}, _b})});

	return false;
}

optional<u256> KnowledgeBase::differenceIfKnownConstant(YulString _a, YulString _b)
{
	return simplify(FunctionCall{{}, {{}, "sub"_yulstring}, util::make_vector<Expression>(Identifier{{}, _a}, Identifier{{}, _b})});
}

bool KnowledgeBase::knownToBeDifferentByAtLeast32(YulString _a, YulString _b)
{
	if (optional<u256> difference = differenceIfKnownConstant(_a, _b))
		return difference >= 32 && difference <= u256(0) - 32;

	return false;
}

bool KnowledgeBase::knownToBeZero(YulString _a)
{
	return valueIfKnownConstant(_a) == u256{};
}

optional<u256> KnowledgeBase::valueIfKnownConstant(YulString _a)
{
	if (AssignedValue const* value = m_variableValues(_a))
		if (Literal const* literal = get_if<Literal>(value->value))
			return valueOfLiteral(*literal);
	return {};
}

optional<u256> KnowledgeBase::simplify(Expression _expression)
{
	if (auto dialect = dynamic_cast<EVMDialect const*>(&m_dialect))
		return SimpleLinearSolver::simplify(*dialect, m_variableValues, _expression);
	else
		return nullopt;
}
