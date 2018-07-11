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
 * @file ExpressionClasses.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Container for equivalence classes of expressions for use in common subexpression elimination.
 */

#include <libevmasm/ExpressionClasses.h>
#include <utility>
#include <functional>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/noncopyable.hpp>
#include <libevmasm/Assembly.h>
#include <libevmasm/CommonSubexpressionEliminator.h>
#include <libevmasm/SimplificationRules.h>

#include <libevmasm/RuleList.h>

using namespace std;
using namespace dev;
using namespace dev::eth;


SimplificationRule<Pattern> const* Rules::findFirstMatch(
	Expression const& _expr,
	ExpressionClasses const& _classes
)
{
	resetMatchGroups();

	assertThrow(_expr.item, OptimizerException, "");
	for (auto const& rule: m_rules[byte(_expr.item->instruction())])
	{
		if (rule.pattern.matches(_expr, _classes))
			return &rule;
		resetMatchGroups();
	}
	return nullptr;
}

void Rules::addRules(std::vector<SimplificationRule<Pattern>> const& _rules)
{
	for (auto const& r: _rules)
		addRule(r);
}

void Rules::addRule(SimplificationRule<Pattern> const& _rule)
{
	m_rules[byte(_rule.pattern.instruction())].push_back(_rule);
}

Rules::Rules()
{
	// Multiple occurrences of one of these inside one rule must match the same equivalence class.
	// Constants.
	Pattern A(Push);
	Pattern B(Push);
	Pattern C(Push);
	// Anything.
	Pattern X;
	Pattern Y;
	A.setMatchGroup(1, m_matchGroups);
	B.setMatchGroup(2, m_matchGroups);
	C.setMatchGroup(3, m_matchGroups);
	X.setMatchGroup(4, m_matchGroups);
	Y.setMatchGroup(5, m_matchGroups);

	addRules(simplificationRuleList(A, B, C, X, Y));
}

Pattern::Pattern(Instruction _instruction, std::vector<Pattern> const& _arguments):
	m_type(Operation),
	m_instruction(_instruction),
	m_arguments(_arguments)
{
}

void Pattern::setMatchGroup(unsigned _group, map<unsigned, Expression const*>& _matchGroups)
{
	m_matchGroup = _group;
	m_matchGroups = &_matchGroups;
}

bool Pattern::matches(Expression const& _expr, ExpressionClasses const& _classes) const
{
	if (!matchesBaseItem(_expr.item))
		return false;
	if (m_matchGroup)
	{
		if (!m_matchGroups->count(m_matchGroup))
			(*m_matchGroups)[m_matchGroup] = &_expr;
		else if ((*m_matchGroups)[m_matchGroup]->id != _expr.id)
			return false;
	}
	assertThrow(m_arguments.size() == 0 || _expr.arguments.size() == m_arguments.size(), OptimizerException, "");
	for (size_t i = 0; i < m_arguments.size(); ++i)
		if (!m_arguments[i].matches(_classes.representative(_expr.arguments[i]), _classes))
			return false;
	return true;
}

AssemblyItem Pattern::toAssemblyItem(SourceLocation const& _location) const
{
	if (m_type == Operation)
		return AssemblyItem(m_instruction, _location);
	else
		return AssemblyItem(m_type, data(), _location);
}

string Pattern::toString() const
{
	stringstream s;
	switch (m_type)
	{
	case Operation:
		s << instructionInfo(m_instruction).name;
		break;
	case Push:
		if (m_data)
			s << "PUSH " << hex << data();
		else
			s << "PUSH ";
		break;
	case UndefinedItem:
		s << "ANY";
		break;
	default:
		if (m_data)
			s << "t=" << dec << m_type << " d=" << hex << data();
		else
			s << "t=" << dec << m_type << " d: nullptr";
		break;
	}
	if (!m_requireDataMatch)
		s << " ~";
	if (m_matchGroup)
		s << "[" << dec << m_matchGroup << "]";
	s << "(";
	for (Pattern const& p: m_arguments)
		s << p.toString() << ", ";
	s << ")";
	return s.str();
}

bool Pattern::matchesBaseItem(AssemblyItem const* _item) const
{
	if (m_type == UndefinedItem)
		return true;
	if (!_item)
		return false;
	if (m_type != _item->type())
		return false;
	else if (m_type == Operation)
		return m_instruction == _item->instruction();
	else if (m_requireDataMatch)
		return data() == _item->data();
	return true;
}

Pattern::Expression const& Pattern::matchGroupValue() const
{
	assertThrow(m_matchGroup > 0, OptimizerException, "");
	assertThrow(!!m_matchGroups, OptimizerException, "");
	assertThrow((*m_matchGroups)[m_matchGroup], OptimizerException, "");
	return *(*m_matchGroups)[m_matchGroup];
}

u256 const& Pattern::data() const
{
	assertThrow(m_data, OptimizerException, "");
	return *m_data;
}

ExpressionTemplate::ExpressionTemplate(Pattern const& _pattern, SourceLocation const& _location)
{
	if (_pattern.matchGroup())
	{
		hasId = true;
		id = _pattern.id();
	}
	else
	{
		hasId = false;
		item = _pattern.toAssemblyItem(_location);
	}
	for (auto const& arg: _pattern.arguments())
		arguments.push_back(ExpressionTemplate(arg, _location));
}

string ExpressionTemplate::toString() const
{
	stringstream s;
	if (hasId)
		s << id;
	else
		s << item;
	s << "(";
	for (auto const& arg: arguments)
		s << arg.toString();
	s << ")";
	return s.str();
}
