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
#include <libyul/optimiser/DataFlowAnalyzer.h>

#include <libsolutil/CommonData.h>

#include <variant>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

bool KnowledgeBase::knownToBeDifferent(YulString _a, YulString _b)
{
	if (optional<u256> difference = differenceIfKnownConstant(_a, _b))
		return difference != 0;
	return false;
}

optional<u256> KnowledgeBase::differenceIfKnownConstant(YulString _a, YulString _b)
{
	VariableOffset offA = explore(_a);
	VariableOffset offB = explore(_b);
	if (offA.reference == offB.reference)
		return offA.offset - offB.offset;
	else
		return {};
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
	VariableOffset offset = explore(_a);
	if (offset.reference == YulString{})
		return offset.offset;
	else
		return nullopt;
}

optional<u256> KnowledgeBase::valueIfKnownConstant(Expression const& _expression)
{
	if (Identifier const* ident = get_if<Identifier>(&_expression))
		return valueIfKnownConstant(ident->name);
	else if (Literal const* lit = get_if<Literal>(&_expression))
		return valueOfLiteral(*lit);
	else
		return {};
}

KnowledgeBase::VariableOffset KnowledgeBase::explore(YulString _var)
{
	// We query the value first so that the variable is reset if it has changed
	// since the last call.
	Expression const* value = valueOf(_var);
	if (VariableOffset const* varOff = util::valueOrNullptr(m_offsets, _var))
		return *varOff;

	if (value)
		if (optional<VariableOffset> offset = explore(*value))
			return setOffset(_var, *offset);
	return setOffset(_var, VariableOffset{_var, 0});

}

optional<KnowledgeBase::VariableOffset> KnowledgeBase::explore(Expression const& _value)
{
	if (Literal const* literal = std::get_if<Literal>(&_value))
		return VariableOffset{YulString{}, valueOfLiteral(*literal)};
	else if (Identifier const* identifier = std::get_if<Identifier>(&_value))
		return explore(identifier->name);
	else if (FunctionCall const* f = get_if<FunctionCall>(&_value))
		if (f->functionName.name == "add"_yulstring || f->functionName.name == "sub"_yulstring)
			if (optional<VariableOffset> a = explore(f->arguments[0]))
				if (optional<VariableOffset> b = explore(f->arguments[1]))
				{
					u256 offset =
						f->functionName.name == "add"_yulstring ?
						a->offset + b->offset :
						a->offset - b->offset;
					if (a->reference == b->reference)
						// Offsets relative to the same reference variable
						return VariableOffset{a->reference, offset};
					else if (a->reference == YulString{})
						// a is constant
						return VariableOffset{b->reference, offset};
					else if (b->reference == YulString{})
						// b is constant
						return VariableOffset{a->reference, offset};
				}

	return {};
}

Expression const* KnowledgeBase::valueOf(YulString _var)
{
	Expression const* lastValue = m_lastKnownValue[_var];
	AssignedValue const* assignedValue = m_variableValues(_var);
	Expression const* currentValue = assignedValue ? assignedValue->value : nullptr;
	if (lastValue != currentValue)
		reset(_var);
	m_lastKnownValue[_var] = currentValue;
	return currentValue;
}

void KnowledgeBase::reset(YulString _var)
{
	m_lastKnownValue.erase(_var);
	if (VariableOffset const* offset = util::valueOrNullptr(m_offsets, _var))
	{
		// Remove var from its group
		if (offset->reference != YulString{})
			m_groupMembers[offset->reference].erase(_var);
		m_offsets.erase(_var);
	}
	if (set<YulString>* group = util::valueOrNullptr(m_groupMembers, _var))
	{
		// _var was a representative, we might have to find a new one.
		if (group->empty())
			m_groupMembers.erase(_var);
		else
		{
			YulString newRepresentative = *group->begin();
			u256 newOffset = m_offsets[newRepresentative].offset;
			for (YulString groupMember: *group)
			{
				yulAssert(m_offsets[groupMember].reference == _var);
				m_offsets[groupMember].reference = newRepresentative;
				m_offsets[newRepresentative].offset -= newOffset;
			}
		}
	}
}

KnowledgeBase::VariableOffset KnowledgeBase::setOffset(YulString _variable, VariableOffset _value)
{
	m_offsets[_variable] = _value;
	// Constants are not tracked in m_groupMembers because
	// the "representative" can never be reset.
	if (_value.reference != YulString{})
		m_groupMembers[_value.reference].insert(_variable);
	return _value;
}
