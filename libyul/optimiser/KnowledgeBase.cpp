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

using namespace solidity;
using namespace solidity::yul;

KnowledgeBase::KnowledgeBase(std::map<YulName, AssignedValue> const& _ssaValues, Dialect const& _dialect):
	m_valuesAreSSA(true),
	m_variableValues([_ssaValues](YulName _var) { return util::valueOrNullptr(_ssaValues, _var); }),
	m_addBuiltinHandle(_dialect.findBuiltin("add")),
	m_subBuiltinHandle(_dialect.findBuiltin("sub"))
{}

bool KnowledgeBase::knownToBeDifferent(YulName _a, YulName _b)
{
	if (std::optional<u256> difference = differenceIfKnownConstant(_a, _b))
		return difference != 0;
	return false;
}

std::optional<u256> KnowledgeBase::differenceIfKnownConstant(YulName _a, YulName _b)
{
	VariableOffset offA = explore(_a);
	VariableOffset offB = explore(_b);
	if (offA.reference == offB.reference)
		return offA.offset - offB.offset;
	else
		return std::nullopt;
}


bool KnowledgeBase::knownToBeDifferentByAtLeast32(YulName _a, YulName _b)
{
	if (std::optional<u256> difference = differenceIfKnownConstant(_a, _b))
		return difference >= 32 && difference <= u256(0) - 32;

	return false;
}

bool KnowledgeBase::knownToBeZero(YulName _a)
{
	return valueIfKnownConstant(_a) == 0;
}

std::optional<u256> KnowledgeBase::valueIfKnownConstant(YulName _a)
{
	return explore(_a).absoluteValue();
}

std::optional<u256> KnowledgeBase::valueIfKnownConstant(Expression const& _expression)
{
	if (Identifier const* ident = std::get_if<Identifier>(&_expression))
		return valueIfKnownConstant(ident->name);
	else if (Literal const* lit = std::get_if<Literal>(&_expression))
		return lit->value.value();
	else
		return std::nullopt;
}

KnowledgeBase::VariableOffset KnowledgeBase::explore(YulName _var)
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
		if (std::optional<VariableOffset> offset = explore(*value))
			return setOffset(_var, *offset);
	return setOffset(_var, VariableOffset{_var, 0});

}

std::optional<KnowledgeBase::VariableOffset> KnowledgeBase::explore(Expression const& _value)
{
	if (Literal const* literal = std::get_if<Literal>(&_value))
		return VariableOffset{YulName{}, literal->value.value()};
	else if (Identifier const* identifier = std::get_if<Identifier>(&_value))
		return explore(identifier->name);
	else if (
		FunctionCall const* f = std::get_if<FunctionCall>(&_value);
		f && isBuiltinFunctionCall(*f)
	)
	{
		if (std::get<BuiltinName>(f->functionName).handle == m_addBuiltinHandle)
		{
			if (std::optional<VariableOffset> a = explore(f->arguments[0]))
				if (std::optional<VariableOffset> b = explore(f->arguments[1]))
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
		else if (std::get<BuiltinName>(f->functionName).handle == m_subBuiltinHandle)
			if (std::optional<VariableOffset> a = explore(f->arguments[0]))
				if (std::optional<VariableOffset> b = explore(f->arguments[1]))
				{
					u256 offset = a->offset - b->offset;
					if (a->reference == b->reference)
						return VariableOffset{YulName{}, offset};
					else if (b->isAbsolute())
						// b is constant
						return VariableOffset{a->reference, offset};
				}
	}

	return std::nullopt;
}

Expression const* KnowledgeBase::valueOf(YulName _var)
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

void KnowledgeBase::reset(YulName _var)
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
	if (std::set<YulName>* group = util::valueOrNullptr(m_groupMembers, _var))
	{
		// _var was a representative, we might have to find a new one.
		if (!group->empty())
		{
			YulName newRepresentative = *group->begin();
			yulAssert(newRepresentative != _var);
			u256 newOffset = m_offsets[newRepresentative].offset;
			// newOffset = newRepresentative - _var
			for (YulName groupMember: *group)
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

KnowledgeBase::VariableOffset KnowledgeBase::setOffset(YulName _variable, VariableOffset _value)
{
	m_offsets[_variable] = _value;
	// Constants are not tracked in m_groupMembers because
	// the "representative" can never be reset.
	if (!_value.reference.empty())
		m_groupMembers[_value.reference].insert(_variable);
	return _value;
}
