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

KnowledgeBase::KnowledgeBase(map<YulString, AssignedValue> const& _ssaValues):
	m_valuesAreSSA(true),
	m_variableValues([_ssaValues](YulString _var) { return util::valueOrNullptr(_ssaValues, _var); })
{}

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
		return nullopt;
}


bool KnowledgeBase::knownToBeDifferentByAtLeast32(YulString _a, YulString _b)
{
	if (optional<u256> difference = differenceIfKnownConstant(_a, _b))
		return difference >= 32 && difference <= u256(0) - 32;

	return false;
}

bool KnowledgeBase::knownToBeZero(YulString _a)
{
	return valueIfKnownConstant(_a) == 0;
}

optional<u256> KnowledgeBase::valueIfKnownConstant(YulString _a)
{
	return explore(_a).absoluteValue();
}

optional<u256> KnowledgeBase::valueIfKnownConstant(Expression const& _expression)
{
	if (Identifier const* ident = get_if<Identifier>(&_expression))
		return valueIfKnownConstant(ident->name);
	else if (Literal const* lit = get_if<Literal>(&_expression))
		return valueOfLiteral(*lit);
	else
		return nullopt;
}

KnowledgeBase::VariableOffset KnowledgeBase::explore(YulString _var)
{
	Expression const* value = nullptr;
	if (m_valuesAreSSA)
	{
		// In SSA, a once determined offset is always valid, so we first see
		// if we already computed it.
		if (VariableOffset const* varOff = util::valueOrNullptr(m_offsets, _var))
			return *varOff;
		value = valueOf(_var);
	}
	else
	{
		// For non-SSA, we query the value first so that the variable is reset if it has changed
		// since the last call.
		value = valueOf(_var);
		if (VariableOffset const* varOff = util::valueOrNullptr(m_offsets, _var))
			return *varOff;
	}

	if (value)
		if (optional<VariableOffset> offset = explore(*value))
			return setOffset(_var, *offset);
	return setOffset(_var, VariableOffset{_var, 0});

}

optional<KnowledgeBase::VariableOffset> KnowledgeBase::explore(Expression const& _value)
{
	if (Literal const* literal = get_if<Literal>(&_value))
		return VariableOffset{YulString{}, valueOfLiteral(*literal)};
	else if (Identifier const* identifier = get_if<Identifier>(&_value))
		return explore(identifier->name);
	else if (FunctionCall const* f = get_if<FunctionCall>(&_value))
	{
		if (f->functionName.name == "add"_yulstring)
		{
			if (optional<VariableOffset> a = explore(f->arguments[0]))
				if (optional<VariableOffset> b = explore(f->arguments[1]))
				{
					u256 offset = a->offset + b->offset;
					if (a->isAbsolute())
						// a is constant
						return VariableOffset{b->reference, offset};
					else if (b->isAbsolute())
						// b is constant
						return VariableOffset{a->reference, offset};
				}
		}
		else if (f->functionName.name == "sub"_yulstring)
			if (optional<VariableOffset> a = explore(f->arguments[0]))
				if (optional<VariableOffset> b = explore(f->arguments[1]))
				{
					u256 offset = a->offset - b->offset;
					if (a->reference == b->reference)
						return VariableOffset{YulString{}, offset};
					else if (b->isAbsolute())
						// b is constant
						return VariableOffset{a->reference, offset};
				}
	}

	return nullopt;
}

Expression const* KnowledgeBase::valueOf(YulString _var)
{
	AssignedValue const* assignedValue = m_variableValues(_var);
	Expression const* currentValue = assignedValue ? assignedValue->value : nullptr;
	if (m_valuesAreSSA)
		return currentValue;

	Expression const* lastValue = m_lastKnownValue[_var];
	if (lastValue != currentValue)
		reset(_var);
	m_lastKnownValue[_var] = currentValue;
	return currentValue;
}

void KnowledgeBase::reset(YulString _var)
{
	yulAssert(!m_valuesAreSSA);

	m_lastKnownValue.erase(_var);
	if (VariableOffset const* offset = util::valueOrNullptr(m_offsets, _var))
	{
		// Remove var from its group
		if (!offset->isAbsolute())
			m_groupMembers[offset->reference].erase(_var);
		m_offsets.erase(_var);
	}
	if (set<YulString>* group = util::valueOrNullptr(m_groupMembers, _var))
	{
		// _var was a representative, we might have to find a new one.
		if (!group->empty())
		{
			YulString newRepresentative = *group->begin();
			yulAssert(newRepresentative != _var);
			u256 newOffset = m_offsets[newRepresentative].offset;
			// newOffset = newRepresentative - _var
			for (YulString groupMember: *group)
			{
				yulAssert(m_offsets[groupMember].reference == _var);
				m_offsets[groupMember].reference = newRepresentative;
				// groupMember = _var + m_offsets[groupMember].offset (old)
				//             = newRepresentative - newOffset + m_offsets[groupMember].offset (old)
				// so subtracting newOffset from .offset yields the original relation again,
				// just with _var replaced by newRepresentative
				m_offsets[groupMember].offset -= newOffset;
			}
			m_groupMembers[newRepresentative] = std::move(*group);

		}
		m_groupMembers.erase(_var);
	}
}

KnowledgeBase::VariableOffset KnowledgeBase::setOffset(YulString _variable, VariableOffset _value)
{
	m_offsets[_variable] = _value;
	// Constants are not tracked in m_groupMembers because
	// the "representative" can never be reset.
	if (!_value.reference.empty())
		m_groupMembers[_value.reference].insert(_variable);
	return _value;
}
