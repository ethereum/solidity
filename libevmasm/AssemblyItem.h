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
/** @file AssemblyItem.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libevmasm/Instruction.h>
#include <libevmasm/Exceptions.h>
#include <liblangutil/SourceLocation.h>
#include <libsolutil/Common.h>
#include <libsolutil/Assertions.h>
#include <iostream>
#include <sstream>

namespace solidity::evmasm
{

enum AssemblyItemType
{
	UndefinedItem,
	Operation,
	Push,
	PushString,
	PushTag,
	PushSub,
	PushSubSize,
	PushProgramSize,
	Tag,
	PushData,
	PushLibraryAddress, ///< Push a currently unknown address of another (library) contract.
	PushDeployTimeAddress, ///< Push an address to be filled at deploy time. Should not be touched by the optimizer.
	PushImmutable, ///< Push the currently unknown value of an immutable variable. The actual value will be filled in by the constructor.
	AssignImmutable, ///< Assigns the current value on the stack to an immutable variable. Only valid during creation code.
	VerbatimBytecode ///< Contains data that is inserted into the bytecode code section without modification.
};

class Assembly;
class AssemblyItem;
using AssemblyItems = std::vector<AssemblyItem>;

class AssemblyItem
{
public:
	enum class JumpType { Ordinary, IntoFunction, OutOfFunction };

	AssemblyItem(u256 _push, langutil::SourceLocation _location = langutil::SourceLocation()):
		AssemblyItem(Push, std::move(_push), std::move(_location)) { }
	/// Used only for the free memory pointer as "memoryguard". Should probably be replaced by a separate AssemblyItemType.
	AssemblyItem(std::shared_ptr<u256> _pushData, langutil::SourceLocation _location = langutil::SourceLocation()):
		m_type(Push), m_data(std::move(_pushData)), m_location(std::move(_location)) { }
	AssemblyItem(Instruction _i, langutil::SourceLocation _location = langutil::SourceLocation()):
		m_type(Operation),
		m_instruction(_i),
		m_location(std::move(_location))
	{}
	AssemblyItem(AssemblyItemType _type, u256 _data = 0, langutil::SourceLocation _location = langutil::SourceLocation()):
		m_type(_type),
		m_location(std::move(_location))
	{
		if (m_type == Operation)
			m_instruction = Instruction(uint8_t(_data));
		else
			m_data = std::make_shared<u256>(std::move(_data));
	}
	explicit AssemblyItem(bytes _verbatimData, size_t _arguments, size_t _returnVariables):
		m_type(VerbatimBytecode),
		m_instruction{},
		m_verbatimBytecode{{_arguments, _returnVariables, std::move(_verbatimData)}}
	{}

	AssemblyItem(AssemblyItem const&) = default;
	AssemblyItem(AssemblyItem&&) = default;
	AssemblyItem& operator=(AssemblyItem const&) = default;
	AssemblyItem& operator=(AssemblyItem&&) = default;

	AssemblyItem tag() const { assertThrow(m_type == PushTag || m_type == Tag, util::Exception, ""); return AssemblyItem(Tag, data()); }
	AssemblyItem pushTag() const { assertThrow(m_type == PushTag || m_type == Tag, util::Exception, ""); return AssemblyItem(PushTag, data()); }
	/// Converts the tag to a subassembly tag. This has to be called in order to move a tag across assemblies.
	/// @param _subId the identifier of the subassembly the tag is taken from.
	AssemblyItem toSubAssemblyTag(size_t _subId) const;
	/// @returns splits the data of the push tag into sub assembly id and actual tag id.
	/// The sub assembly id of non-foreign push tags is -1.
	std::pair<size_t, size_t> splitForeignPushTag() const;
	/// Sets sub-assembly part and tag for a push tag.
	void setPushTagSubIdAndTag(size_t _subId, size_t _tag);

	AssemblyItemType type() const { return m_type; }
	u256 const& data() const { assertThrow(m_type != Operation, util::Exception, ""); return *m_data; }
	void setData(u256 const& _data) { assertThrow(m_type != Operation, util::Exception, ""); m_data = std::make_shared<u256>(_data); }

	bytes const& verbatimData() const { assertThrow(m_type == VerbatimBytecode, util::Exception, ""); return std::get<2>(*m_verbatimBytecode); }

	/// @returns the instruction of this item (only valid if type() == Operation)
	Instruction instruction() const { assertThrow(m_type == Operation, util::Exception, ""); return m_instruction; }

	/// @returns true if the type and data of the items are equal.
	bool operator==(AssemblyItem const& _other) const
	{
		if (type() != _other.type())
			return false;
		if (type() == Operation)
			return instruction() == _other.instruction();
		else if (type() == VerbatimBytecode)
			return *m_verbatimBytecode == *_other.m_verbatimBytecode;
		else
			return data() == _other.data();
	}
	bool operator!=(AssemblyItem const& _other) const { return !operator==(_other); }
	/// Less-than operator compatible with operator==.
	bool operator<(AssemblyItem const& _other) const
	{
		if (type() != _other.type())
			return type() < _other.type();
		else if (type() == Operation)
			return instruction() < _other.instruction();
		else if (type() == VerbatimBytecode)
			return *m_verbatimBytecode == *_other.m_verbatimBytecode;
		else
			return data() < _other.data();
	}

	/// Shortcut that avoids constructing an AssemblyItem just to perform the comparison.
	bool operator==(Instruction _instr) const
	{
		return type() == Operation && instruction() == _instr;
	}
	bool operator!=(Instruction _instr) const { return !operator==(_instr); }

	static std::string computeSourceMapping(
		AssemblyItems const& _items,
		std::map<std::string, unsigned> const& _sourceIndicesMap
	);

	/// @returns an upper bound for the number of bytes required by this item, assuming that
	/// the value of a jump tag takes @a _addressLength bytes.
	size_t bytesRequired(size_t _addressLength) const;
	size_t arguments() const;
	size_t returnValues() const;
	size_t deposit() const { return returnValues() - arguments(); }

	/// @returns true if the assembly item can be used in a functional context.
	bool canBeFunctional() const;

	void setLocation(langutil::SourceLocation const& _location) { m_location = _location; }
	langutil::SourceLocation const& location() const { return m_location; }

	void setJumpType(JumpType _jumpType) { m_jumpType = _jumpType; }
	JumpType getJumpType() const { return m_jumpType; }
	std::string getJumpTypeAsString() const;

	void setPushedValue(u256 const& _value) const { m_pushedValue = std::make_shared<u256>(_value); }
	u256 const* pushedValue() const { return m_pushedValue.get(); }

	std::string toAssemblyText(Assembly const& _assembly) const;

	size_t m_modifierDepth = 0;

	void setImmutableOccurrences(size_t _n) const { m_immutableOccurrences = std::make_shared<size_t>(_n); }

private:
	AssemblyItemType m_type;
	Instruction m_instruction; ///< Only valid if m_type == Operation
	std::shared_ptr<u256> m_data; ///< Only valid if m_type != Operation
	/// If m_type == VerbatimBytecode, this holds number of arguments, number of
	/// return variables and verbatim bytecode.
	std::optional<std::tuple<size_t, size_t, bytes>> m_verbatimBytecode;
	langutil::SourceLocation m_location;
	JumpType m_jumpType = JumpType::Ordinary;
	/// Pushed value for operations with data to be determined during assembly stage,
	/// e.g. PushSubSize, PushTag, PushSub, etc.
	mutable std::shared_ptr<u256> m_pushedValue;
	/// Number of PushImmutable's with the same hash. Only used for AssignImmutable.
	mutable std::shared_ptr<size_t> m_immutableOccurrences;
};

inline size_t bytesRequired(AssemblyItems const& _items, size_t _addressLength)
{
	size_t size = 0;
	for (AssemblyItem const& item: _items)
		size += item.bytesRequired(_addressLength);
	return size;
}

std::ostream& operator<<(std::ostream& _out, AssemblyItem const& _item);
inline std::ostream& operator<<(std::ostream& _out, AssemblyItems const& _items)
{
	for (AssemblyItem const& item: _items)
		_out << item;
	return _out;
}

}
