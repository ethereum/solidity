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
 * @author Rhett <roadriverrail@gmail.com>
 * @date 2017
 * Overflow analyzer and checker.
 */

#include <libsolidity/analysis/OverflowChecker.h>
#include <libsolidity/analysis/IntegerOverflowBounds.h>
#include <memory>
#include <boost/range/adaptor/reversed.hpp>
#include <libsolidity/ast/AST.h>
#include <libsolidity/inlineasm/AsmAnalysis.h>
#include <libsolidity/inlineasm/AsmAnalysisInfo.h>
#include <libsolidity/inlineasm/AsmData.h>
#include <libsolidity/interface/ErrorReporter.h>
#include <iostream>
#include <typeinfo>
using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace boost::multiprecision;

void OverflowChecker::checkOverflow(SourceUnit const& _source)
{
	try
	{
		bool pragmaFound = false;
		for (auto const& node: _source.nodes())
			if (auto const* pragma = dynamic_cast<PragmaDirective const*>(node.get()))
				if (pragma->literals()[0] == "analyzeIntegerOverflow")
					pragmaFound = true;
		if (pragmaFound)
			_source.accept(*this);
	}
	catch (FatalError const&)
	{
		// We got a fatal error which required to stop further checking, but we can
		// continue normally from here.
		if (m_errorReporter.errors().empty())
			throw; // Something is weird here, rather throw again.
	}
}

bool OverflowChecker::visit(FunctionDefinition const&)
{
	// Clear out the overflow info state before proceeding.
	m_overflowInfo.clear();
	m_varOverflowInfo.clear();
	m_constraints.clear();
	return true;
}

bool OverflowChecker::visit(Assignment const& _assignment)
{
	_assignment.rightHandSide().accept(*this);
	auto const identifier = dynamic_cast<Identifier const*>(&_assignment.leftHandSide());
	if (identifier)
	{
		IntegerOverflowBounds bounds = m_overflowInfo[&_assignment.rightHandSide()];
		auto var = dynamic_cast<VariableDeclaration const*>(identifier->annotation().referencedDeclaration);
		solAssert(var, string("Failed to find variable declaration for ") + identifier->name());
		m_varOverflowInfo[var] = bounds;
		if (!bounds.fitsType(*(var->annotation().type)))
			m_errorReporter.warning(_assignment.location(), "Variable assignment may cause overflow.  " + bounds.toString());
	}

	return false;
}

bool OverflowChecker::lockConstantExpression(Expression const& _expression)
{
	auto const* rationalNumberType = dynamic_cast<RationalNumberType const*>(_expression.annotation().type.get());
	if (rationalNumberType)
	{
		m_overflowInfo[&_expression] = boundsForType(*rationalNumberType);
		return true;
	}
	return false;
}

bool OverflowChecker::visit(Literal const& _literal)
{
	lockConstantExpression(_literal);
	return true;
}

bool OverflowChecker::visit(BinaryOperation const& _operation)
{
	if (lockConstantExpression(_operation))
		return false; // Do not descend

	// Need to do more work...
	auto const* integerType = dynamic_cast<IntegerType const*>(_operation.annotation().commonType.get());
	if (integerType)
	{
		//First, descend and resolve both sides of the operation.
		_operation.leftExpression().accept(*this);
		_operation.rightExpression().accept(*this);

		//If the operator is a comparison, then generate any possible constraints.
		//The most naive form of this will only work if the LHS is an identifier.
		if (Token::isCompareOp(_operation.getOperator()))
		{
			auto const identifier = dynamic_cast<Identifier const*>(&_operation.leftExpression());
			auto const rightIdentifier = dynamic_cast<Identifier const*>(&_operation.rightExpression());
			if (identifier)
			{
				auto var = dynamic_cast<VariableDeclaration const*>(identifier->annotation().referencedDeclaration);
				if (var)
				{
					IntegerOverflowBounds constraint = boundsForType(*_operation.leftExpression().annotation().type);
					m_constraints[var] = performBinaryOp(_operation.getOperator(),
							constraint,
							m_overflowInfo[&(_operation.rightExpression())]);
					return true;
				}
			}
			else if (rightIdentifier)
			{
				// Something was in the format of (5 > x) instead (x < 5)
				auto var = dynamic_cast<VariableDeclaration const*>(rightIdentifier->annotation().referencedDeclaration);
				if (var)
				{
					Token::Value invertOp;
					switch (_operation.getOperator())
					{
						case Token::LessThan:
							invertOp = Token::GreaterThan;
							break;
						case Token::GreaterThan:
							invertOp = Token::LessThan;
							break;
						case Token::Equal:
							invertOp = Token::Equal;
							break;
						default:
							break;
					}
					// Ignoring <= and => just for the sake of simplicity for now.  == is symmetric
					IntegerOverflowBounds constraint = boundsForType(*_operation.rightExpression().annotation().type);
					m_constraints[var] = performBinaryOp(invertOp,
							constraint,
							m_overflowInfo[&(_operation.leftExpression())]);
					return true;
				}
			}
		}
		else
		{
			IntegerOverflowBounds leftBounds = m_overflowInfo[&(_operation.leftExpression())];
			IntegerOverflowBounds rightBounds = m_overflowInfo[&(_operation.rightExpression())];

			m_overflowInfo[&_operation] = performBinaryOp(_operation.getOperator(), leftBounds, rightBounds);
			if (!m_overflowInfo[&_operation].fitsType(*integerType))
				m_errorReporter.warning(_operation.location(), "Expression potentially overflowing.  Range is: " + m_overflowInfo[&_operation].toString() + " Allowed range is: " + boundsForType(*integerType).toString());
			return false;
		}

	}
	return true;
}

bool OverflowChecker::visit(UnaryOperation const& _operation)
{
	if (lockConstantExpression(_operation))
		return false; // Do not descend
	else
		// Need to do more work...
	return true;
}

bool OverflowChecker::visit(VariableDeclaration const& _variable)
{
	auto const* integerType = dynamic_cast<IntegerType const*>(_variable.annotation().type.get());
	if (integerType)
	{
		IntegerOverflowBounds bounds = boundsForType(*integerType);
		if (_variable.isLocalOrReturn())
			bounds = zeroBounds();

		m_varOverflowInfo[&_variable] = bounds;
	}
	return true;
}

bool OverflowChecker::visit(VariableDeclarationStatement const& _statement)
{
	// Resolve the expression
	if (_statement.initialValue())
		_statement.initialValue()->accept(*this);
	// Store and assign all the variables
	for (auto var : _statement.declarations())
	{
		var->accept(*this);
		if (_statement.initialValue())
		{
			IntegerOverflowBounds initialBounds = m_overflowInfo[_statement.initialValue()];
			m_varOverflowInfo[var.get()] = initialBounds;
			if (!initialBounds.fitsType(*(var->type())))
				m_errorReporter.warning(_statement.location(), "Assignment may overflow.  Range: "+initialBounds.toString()+" Allowed range: "+boundsForType(*var->type()).toString());
		}
	}
	// Picked this statement clean; don't descend
	return false;
}

bool OverflowChecker::visit(Identifier const& _identifier)
{
	auto var = dynamic_cast<VariableDeclaration const*>(_identifier.annotation().referencedDeclaration);
	if (var)
	{
		m_overflowInfo[&_identifier] = m_varOverflowInfo[var];
	}
	return false;
}

bool OverflowChecker::visit(FunctionCall const& _functionCall)
{
	FunctionTypePointer functionType = dynamic_pointer_cast<FunctionType const>(_functionCall.expression().annotation().type);
	if (functionType)
	{
		if (functionType->kind() == FunctionType::Kind::Require || functionType->kind() == FunctionType::Kind::Assert)
		// As we know that code beyond this function call must be constrained by the
		// assert/require, we can limit the ranges of the values inside.
		// So, we need to descend the expression, collect the constraints, and then
		// apply them.
		//
		// I think it might be enough to associate each constraint with an OverflowBounds and
		// then intersect them against the existing ones.
		{
			m_constraints.clear();
			_functionCall.arguments().front()->accept(*this);
			for(auto const& constraint : m_constraints)
			{
				m_varOverflowInfo[constraint.first] = intersectBounds(m_varOverflowInfo[constraint.first], constraint.second);
			}
		}
	}
	else
	{
		//Descend arguments.
		for (auto arg : _functionCall.arguments())
			arg->accept(*this);
		auto const* integerType = dynamic_cast<IntegerType const*>(_functionCall.annotation().type.get());
		if (integerType)
		{
			// It's probably possible to be less naive about function returns, but this will
			// at least get the ball rolling
			m_overflowInfo[&_functionCall] = boundsForType(*integerType);
		}

	}
	return true;
}

bool OverflowChecker::visit(IfStatement const& _ifStatement)
{
	// We will naively assume that all conditions are non-taugological

	// Then we must make a copy for the if block and the else block
	std::map<VariableDeclaration const*, IntegerOverflowBounds> trueBounds;
	std::map<VariableDeclaration const*, IntegerOverflowBounds> falseBounds;
	std::map<VariableDeclaration const*, IntegerOverflowBounds> swap;

	// Consider a copy construction of the maps?
	for (auto const& varBounds : m_varOverflowInfo)
	{
		trueBounds[varBounds.first] = varBounds.second;
		falseBounds[varBounds.first] = varBounds.second;
	}

	// Visiting the condition will generate the constraints
	m_constraints.clear();
	_ifStatement.condition().accept(*this);

	for (auto const& constraint : m_constraints)
	{
		// Note that the boolean statement from the condition must also be
		// inverted for the else block

		trueBounds[constraint.first] = intersectBounds(trueBounds[constraint.first], constraint.second);

		// FIXME: If we say if (x < 2) and x is a uint8, then the constrained bounds for
		// this statement to be true is [0,1] and its inversion is [2,255].  Even putting aside
		// that the IntegerOverflowBounds object can't handle a disjoint range currently, and thus
		// it's not possible to invert a range like [1,2], we still need to pass in a type to the
		// inversion to know what the real max and min values of the target range are.  This means
		// that a bound can't itself be inverted without type information present, which feels like
		// it defeats the point of not tying a range object to a type in the first place.
		IntegerOverflowBounds invertedBounds = invertBounds(constraint.second, *constraint.first->annotation().type);
		falseBounds[constraint.first] = intersectBounds(falseBounds[constraint.first], invertedBounds);
	}

	// Each block is evaluated with its own copy of the overflow info
	swap = m_varOverflowInfo;
        m_varOverflowInfo = trueBounds;
	_ifStatement.trueStatement().accept(*this);
	trueBounds = m_varOverflowInfo;

	m_varOverflowInfo = falseBounds;
	if (_ifStatement.falseStatement())
		_ifStatement.falseStatement()->accept(*this);
	falseBounds = m_varOverflowInfo;

	m_varOverflowInfo = swap;

	// At the end, union the resulting conditions together.  Note that we don't actually care about any of the
	// variables introduced in the if/else blocks because they are not in scope now.
	for (auto const& varBounds : m_varOverflowInfo)
		m_varOverflowInfo[varBounds.first] = unionBounds(trueBounds[varBounds.first], falseBounds[varBounds.first]);

	// Do not auto-descend; everything was visited
	return false;
}
