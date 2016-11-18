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
/** @file Assembly.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <iostream>
#include <sstream>
#include <libdevcore/Common.h>
#include <libdevcore/Assertions.h>
#include <libevmasm/Instruction.h>
#include <libevmasm/SourceLocation.h>
#include "Exceptions.h"
using namespace dev::solidity;

namespace dev
{
namespace eth
{

enum AssemblyItemType {
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
	PushLibraryAddress ///< Push a currently unknown address of another (library) contract.
};

class Assembly;

class AssemblyItem
{
public:
	enum class JumpType { Ordinary, IntoFunction, OutOfFunction };

	AssemblyItem(u256 _push, SourceLocation const& _location = SourceLocation()):
		AssemblyItem(Push, _push, _location) { }
	AssemblyItem(solidity::Instruction _i, SourceLocation const& _location = SourceLocation()):
		AssemblyItem(Operation, byte(_i), _location) { }
	AssemblyItem(AssemblyItemType _type, u256 _data = 0, SourceLocation const& _location = SourceLocation()):
		m_type(_type),
		m_data(_data),
		m_location(_location)
	{
	}

	AssemblyItem tag() const { assertThrow(m_type == PushTag || m_type == Tag, Exception, ""); return AssemblyItem(Tag, m_data); }
	AssemblyItem pushTag() const { assertThrow(m_type == PushTag || m_type == Tag, Exception, ""); return AssemblyItem(PushTag, m_data); }
	/// Converts the tag to a subassembly tag. This has to be called in order to move a tag across assemblies.
	/// @param _subId the identifier of the subassembly the tag is taken from.
	AssemblyItem toSubAssemblyTag(size_t _subId) const;
	/// @returns splits the data of the push tag into sub assembly id and actual tag id.
	/// The sub assembly id of non-foreign push tags is -1.
	std::pair<size_t, size_t> splitForeignPushTag() const;
	/// Sets sub-assembly part and tag for a push tag.
	void setPushTagSubIdAndTag(size_t _subId, size_t _tag);

	AssemblyItemType type() const { return m_type; }
	u256 const& data() const { return m_data; }
	void setType(AssemblyItemType const _type) { m_type = _type; }
	void setData(u256 const& _data) { m_data = _data; }

	/// @returns the instruction of this item (only valid if type() == Operation)
	Instruction instruction() const { return Instruction(byte(m_data)); }

	/// @returns true if the type and data of the items are equal.
	bool operator==(AssemblyItem const& _other) const { return m_type == _other.m_type && m_data == _other.m_data; }
	bool operator!=(AssemblyItem const& _other) const { return !operator==(_other); }
	/// Less-than operator compatible with operator==.
	bool operator<(AssemblyItem const& _other) const { return std::tie(m_type, m_data) < std::tie(_other.m_type, _other.m_data); }

	/// @returns an upper bound for the number of bytes required by this item, assuming that
	/// the value of a jump tag takes @a _addressLength bytes.
	unsigned bytesRequired(unsigned _addressLength) const;
	int deposit() const;

	bool match(AssemblyItem const& _i) const { return _i.m_type == UndefinedItem || (m_type == _i.m_type && (m_type != Operation || m_data == _i.m_data)); }
	void setLocation(SourceLocation const& _location) { m_location = _location; }
	SourceLocation const& location() const { return m_location; }

	void setJumpType(JumpType _jumpType) { m_jumpType = _jumpType; }
	JumpType getJumpType() const { return m_jumpType; }
	std::string getJumpTypeAsString() const;

	void setPushedValue(u256 const& _value) const { m_pushedValue = std::make_shared<u256>(_value); }
	u256 const* pushedValue() const { return m_pushedValue.get(); }

private:
	AssemblyItemType m_type;
	u256 m_data;
	SourceLocation m_location;
	JumpType m_jumpType = JumpType::Ordinary;
	/// Pushed value for operations with data to be determined during assembly stage,
	/// e.g. PushSubSize, PushTag, PushSub, etc.
	mutable std::shared_ptr<u256> m_pushedValue;
};

using AssemblyItems = std::vector<AssemblyItem>;

std::ostream& operator<<(std::ostream& _out, AssemblyItem const& _item);
inline std::ostream& operator<<(std::ostream& _out, AssemblyItems const& _items)
{
	for (AssemblyItem const& item: _items)
		_out << item;
	return _out;
}

}
}
