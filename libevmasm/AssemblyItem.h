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
#include <libevmasm/SubAssemblyID.h>
#include <liblangutil/DebugData.h>
#include <liblangutil/Exceptions.h>
#include <libsolutil/Common.h>
#include <libsolutil/Numeric.h>
#include <libsolutil/Assertions.h>

#include <boost/container_hash/hash.hpp>

#include <optional>
#include <iostream>
#include <sstream>
#include <type_traits>

namespace solidity::evmasm
{

enum AssemblyItemType
{
	UndefinedItem,
	Operation,
	Push,
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

	VerbatimBytecode, ///< Contains data that is inserted into the bytecode code section without modification.
};

enum class Precision { Precise , Approximate };

class Assembly;
class AssemblyItem;
using AssemblyItems = std::vector<AssemblyItem>;
using ContainerID = uint8_t;

class AssemblyItem
{
public:
	enum class JumpType { Ordinary, IntoFunction, OutOfFunction };

	AssemblyItem(u256 _push, langutil::DebugData::ConstPtr _debugData = langutil::DebugData::create()):
		AssemblyItem(Push, std::move(_push), std::move(_debugData)) { }
	AssemblyItem(Instruction _i, langutil::DebugData::ConstPtr _debugData = langutil::DebugData::create()):
		m_type(Operation),
		m_instruction(_i),
		m_debugData(std::move(_debugData))
	{}
	AssemblyItem(AssemblyItemType _type, u256 _data = 0, langutil::DebugData::ConstPtr _debugData = langutil::DebugData::create()):
		m_type(_type),
		m_debugData(std::move(_debugData))
	{
		if (m_type == Operation)
			m_instruction = Instruction(uint8_t(_data));
		else
			m_data = std::make_shared<u256>(std::move(_data));
	}

	explicit AssemblyItem(AssemblyItemType _type, Instruction _instruction, u256 _data = 0, langutil::DebugData::ConstPtr _debugData = langutil::DebugData::create()):
		m_type(_type),
		m_instruction(_instruction),
		m_data(std::make_shared<u256>(std::move(_data))),
		m_debugData(std::move(_debugData))
	{}

	explicit AssemblyItem(bytes _verbatimData, size_t _arguments, size_t _returnVariables):
		m_type(VerbatimBytecode),
		m_verbatimBytecode{{_arguments, _returnVariables, std::move(_verbatimData)}},
		m_debugData{langutil::DebugData::create()}
	{}

	AssemblyItem(AssemblyItem const&) = default;
	AssemblyItem(AssemblyItem&&) = default;
	AssemblyItem& operator=(AssemblyItem const&) = default;
	AssemblyItem& operator=(AssemblyItem&&) = default;

	AssemblyItem tag() const { solAssert(m_type == PushTag || m_type == Tag); return AssemblyItem(Tag, data()); }
	AssemblyItem pushTag() const { solAssert(m_type == PushTag || m_type == Tag); return AssemblyItem(PushTag, data()); }
	/// Converts the tag to a subassembly tag. This has to be called in order to move a tag across assemblies.
	/// @param _subId the identifier of the subassembly the tag is taken from.
	AssemblyItem toSubAssemblyTag(SubAssemblyID _subId) const;
	/// @returns splits the data of the push tag into sub assembly id and actual tag id.
	/// The sub assembly id of non-foreign push tags is -1.
	std::pair<SubAssemblyID, size_t> splitForeignPushTag() const;
	/// @returns relative jump target tag ID. Asserts that it is not foreign tag.
	size_t relativeJumpTagID() const;
	/// Sets sub-assembly part and tag for a push tag.
	void setPushTagSubIdAndTag(SubAssemblyID _subId, size_t _tag);

	AssemblyItemType type() const { return m_type; }
	u256 const& data() const { solAssert(m_type != Operation && m_data != nullptr); return *m_data; }
	void setData(u256 const& _data) { assertThrow(m_type != Operation, util::Exception, ""); m_data = std::make_shared<u256>(_data); }

	/// This function is used in `Assembly::assemblyJSON`.
	/// It returns the name & data of the current assembly item.
	/// @param _evmVersion the EVM version.
	/// @returns a pair, where the first element is the json-assembly
	/// item name, where second element is the string representation
	/// of it's data.
	std::pair<std::string, std::string> nameAndData(langutil::EVMVersion _evmVersion) const;

	bytes const& verbatimData() const { assertThrow(m_type == VerbatimBytecode, util::Exception, ""); return std::get<2>(*m_verbatimBytecode); }

	/// @returns true if the item has m_instruction properly set.
	bool hasInstruction() const
	{
		bool const shouldHaveInstruction =
			m_type == Operation;
		solAssert(shouldHaveInstruction == m_instruction.has_value());
		return shouldHaveInstruction;
	}
	/// @returns the instruction of this item (only valid if hasInstruction returns true)
	Instruction instruction() const
	{
		solAssert(hasInstruction());
		return *m_instruction;
	}

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
	/// Hash function compatible with `operator==`. Found via ADL by `boost::hash`.
	friend std::size_t hash_value(AssemblyItem const& _item)
	{
		std::size_t hash = 0;
		boost::hash_combine(hash, static_cast<std::underlying_type_t<AssemblyItemType>>(_item.m_type));
		if (_item.m_type == Operation)
			boost::hash_combine(hash, static_cast<std::underlying_type_t<Instruction>>(_item.instruction()));
		else if (_item.m_type == VerbatimBytecode)
			boost::hash_combine(hash, *_item.m_verbatimBytecode);
		else
			boost::hash_combine(hash, _item.data());
		return hash;
	}
	/// Less-than operator compatible with operator==.
	bool operator<(AssemblyItem const& _other) const
	{
		if (type() != _other.type())
			return type() < _other.type();
		else if (type() == Operation)
			return instruction() < _other.instruction();
		else if (type() == VerbatimBytecode)
			return *m_verbatimBytecode < *_other.m_verbatimBytecode;
		else
			return data() < _other.data();
	}

	/// Shortcut that avoids constructing an AssemblyItem just to perform the comparison.
	bool operator==(Instruction _instr) const
	{
		return hasInstruction() && instruction() == _instr;
	}
	bool operator!=(Instruction _instr) const { return !operator==(_instr); }

	static std::string computeSourceMapping(
		AssemblyItems const& _items,
		std::map<std::string, unsigned> const& _sourceIndicesMap
	);

	/// @returns an upper bound for the number of bytes required by this item, assuming that
	/// the value of a jump tag takes @a _addressLength bytes.
	/// @param _evmVersion the EVM version
	/// @param _precision Whether to return a precise count (which involves
	///                   counting immutable references which are only set after
	///                   a call to `assemble()`) or an approx. count.
	size_t bytesRequired(size_t _addressLength, langutil::EVMVersion _evmVersion, Precision _precision = Precision::Precise) const;
	size_t arguments() const;
	size_t returnValues() const;
	size_t deposit() const { return returnValues() - arguments(); }

	/// @returns true if the assembly item can be used in a functional context.
	bool canBeFunctional() const;

	void setLocation(langutil::SourceLocation const& _location)
	{
		solAssert(m_debugData);
		m_debugData = langutil::DebugData::create(
			_location,
			m_debugData->originLocation,
			m_debugData->astID
		);
	}

	langutil::SourceLocation const& location() const
	{
		solAssert(m_debugData);
		return m_debugData->nativeLocation;
	}

	void setDebugData(langutil::DebugData::ConstPtr _debugData)
	{
		solAssert(_debugData);
		m_debugData = std::move(_debugData);
	}

	langutil::DebugData::ConstPtr debugData() const { return m_debugData; }

	void setJumpType(JumpType _jumpType) { m_jumpType = _jumpType; }
	/// Marks a Tag item as a subroutine entry: it is assembled as CALLDEST instead of JUMPDEST (EIP-7979).
	void setSubroutineEntry() { solAssert(m_type == Tag); m_subroutineEntry = true; }
	/// @returns true if this is a Tag item that is assembled as CALLDEST (EIP-7979).
	bool isSubroutineEntry() const { return m_type == Tag && m_subroutineEntry; }
	static std::optional<JumpType> parseJumpType(std::string const& _jumpType);
	JumpType getJumpType() const { return m_jumpType; }
	std::string getJumpTypeAsString() const;

	void setPushedValue(u256 const& _value) const { m_pushedValue = std::make_shared<u256>(_value); }
	u256 const* pushedValue() const { return m_pushedValue.get(); }

	std::string toAssemblyText(Assembly const& _assembly) const;

	size_t m_modifierDepth = 0;

	void setImmutableOccurrences(size_t _n) const { m_immutableOccurrences = _n; }

private:
	size_t opcodeCount() const noexcept;

	AssemblyItemType m_type;
	std::optional<Instruction> m_instruction; ///< Only valid for item types that represent a specific opcode
	bool m_subroutineEntry = false; ///< Only valid for Tag: assemble as CALLDEST (EIP-7979)
	std::shared_ptr<u256> m_data; ///< Only valid if m_type != Operation
	/// If m_type == VerbatimBytecode, this holds number of arguments, number of
	/// return variables and verbatim bytecode.
	std::optional<std::tuple<size_t, size_t, bytes>> m_verbatimBytecode;
	langutil::DebugData::ConstPtr m_debugData;
	JumpType m_jumpType = JumpType::Ordinary;
	/// Pushed value for operations with data to be determined during assembly stage,
	/// e.g. PushSubSize, PushTag, PushSub, etc.
	mutable std::shared_ptr<u256> m_pushedValue;
	/// Number of PushImmutable's with the same hash. Only used for AssignImmutable.
	mutable std::optional<size_t> m_immutableOccurrences;
};

inline size_t bytesRequired(AssemblyItems const& _items, size_t _addressLength, langutil::EVMVersion _evmVersion, Precision _precision = Precision::Precise)
{
	size_t size = 0;
	for (AssemblyItem const& item: _items)
		size += item.bytesRequired(_addressLength, _evmVersion, _precision);
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
