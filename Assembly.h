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
/** @file CodeFragment.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libethsupport/Common.h>
#include <libethcore/Instruction.h>
#include "Exceptions.h"

namespace eth
{

enum AssemblyItemType { Operation, Push, PushString, PushTag, Tag, PushData };

class AssemblyItem
{
public:
	AssemblyItem(u256 _push): m_type(Push), m_data(_push) {}
	AssemblyItem(std::string const& _push): m_type(PushString), m_pushString(_push) {}
	AssemblyItem(AssemblyItemType _type, AssemblyItem const& _tag): m_type(_type), m_data(_tag.m_data) { assert(_type == PushTag); assert(_tag.m_type == Tag); }
	AssemblyItem(Instruction _i): m_type(Operation), m_data((byte)_i) {}
	AssemblyItem(AssemblyItemType _type, u256 _data): m_type(_type), m_data(_data) {}

	AssemblyItemType type() const { return m_type; }
	u256 data() const { return m_data; }
	std::string const& pushString() const { return m_pushString; }

private:
	AssemblyItemType m_type;
	u256 m_data;
	std::string m_pushString;
};

class Assembly
{
public:
	AssemblyItem newTag() { return AssemblyItem(Tag, m_usedTags++); }
	AssemblyItem newData(bytes const& _data) { auto h = sha3(_data); m_data[h] = _data; return AssemblyItem(PushData, h); }
	bytes assemble() const;
	void append(Assembly const& _a);

private:
	u256 m_usedTags = 0;
	std::vector<AssemblyItem> m_items;
	std::map<h256, bytes> m_data;
};

}
