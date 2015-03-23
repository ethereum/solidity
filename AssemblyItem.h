/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
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
#include <libevmcore/SourceLocation.h>
#include <libevmcore/Instruction.h>
#include "Exceptions.h"

namespace dev
{
namespace eth
{

enum AssemblyItemType { UndefinedItem, Operation, Push, PushString, PushTag, PushSub, PushSubSize, PushProgramSize, Tag, PushData };

class Assembly;

class AssemblyItem
{
	friend class Assembly;

public:
	enum class JumpType { Ordinary, IntoFunction, OutOfFunction };

	AssemblyItem(u256 _push): m_type(Push), m_data(_push) {}
	AssemblyItem(Instruction _i): m_type(Operation), m_data((byte)_i) {}
	AssemblyItem(AssemblyItemType _type, u256 _data = 0): m_type(_type), m_data(_data) {}

	AssemblyItem tag() const { assertThrow(m_type == PushTag || m_type == Tag, Exception, ""); return AssemblyItem(Tag, m_data); }
	AssemblyItem pushTag() const { assertThrow(m_type == PushTag || m_type == Tag, Exception, ""); return AssemblyItem(PushTag, m_data); }

	AssemblyItemType type() const { return m_type; }
	u256 const& data() const { return m_data; }
	/// @returns the instruction of this item (only valid if type() == Operation)
	Instruction instruction() const { return Instruction(byte(m_data)); }

	/// @returns true iff the type and data of the items are equal.
	bool operator==(AssemblyItem const& _other) const { return m_type == _other.m_type && m_data == _other.m_data; }
	bool operator!=(AssemblyItem const& _other) const { return !operator==(_other); }

	/// @returns an upper bound for the number of bytes required by this item, assuming that
	/// the value of a jump tag takes @a _addressLength bytes.
	unsigned bytesRequired(unsigned _addressLength) const;
	int deposit() const;

	bool match(AssemblyItem const& _i) const { return _i.m_type == UndefinedItem || (m_type == _i.m_type && (m_type != Operation || m_data == _i.m_data)); }
	void setLocation(SourceLocation const& _location) { m_location = _location; }
	SourceLocation const& getLocation() const { return m_location; }

	void setJumpType(JumpType _jumpType) { m_jumpType = _jumpType; }
	JumpType getJumpType() const { return m_jumpType; }
	std::string getJumpTypeAsString() const;

private:
	AssemblyItemType m_type;
	u256 m_data;
	SourceLocation m_location;
	JumpType m_jumpType = JumpType::Ordinary;
};

using AssemblyItems = std::vector<AssemblyItem>;
using AssemblyItemsConstRef = vector_ref<AssemblyItem const>;

std::ostream& operator<<(std::ostream& _out, AssemblyItem const& _item);
std::ostream& operator<<(std::ostream& _out, AssemblyItemsConstRef _i);
inline std::ostream& operator<<(std::ostream& _out, AssemblyItems const& _i) { return operator<<(_out, AssemblyItemsConstRef(&_i)); }

}
}
