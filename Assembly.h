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

#include <iostream>
#include <sstream>
#include <libethsupport/Common.h>
#include <libethcore/Instruction.h>
#include "Exceptions.h"

namespace eth
{

enum AssemblyItemType { UndefinedItem, Operation, Push, PushString, PushTag, Tag, PushData };

class Assembly;

class AssemblyItem
{
	friend class Assembly;

public:
	AssemblyItem(u256 _push): m_type(Push), m_data(_push) {}
	AssemblyItem(Instruction _i): m_type(Operation), m_data((byte)_i) {}
	AssemblyItem(AssemblyItemType _type, u256 _data = 0): m_type(_type), m_data(_data) {}

	AssemblyItem tag() const { assert(m_type == PushTag || m_type == Tag); return AssemblyItem(Tag, m_data); }
	AssemblyItem pushTag() const { assert(m_type == PushTag || m_type == Tag); return AssemblyItem(PushTag, m_data); }

	AssemblyItemType type() const { return m_type; }
	u256 data() const { return m_data; }

	int deposit() const;

	bool match(AssemblyItem const& _i) const { return _i.m_type == UndefinedItem || (m_type == _i.m_type && (m_type != Operation || m_data == _i.m_data)); }

private:
	AssemblyItemType m_type;
	u256 m_data;
};

typedef std::vector<AssemblyItem> AssemblyItems;
typedef vector_ref<AssemblyItem const> AssemblyItemsConstRef;

std::ostream& operator<<(std::ostream& _out, AssemblyItemsConstRef _i);

class Assembly
{
public:
	AssemblyItem newTag() { return AssemblyItem(Tag, m_usedTags++); }
	AssemblyItem newPushTag() { return AssemblyItem(PushTag, m_usedTags++); }
	AssemblyItem newData(bytes const& _data) { auto h = sha3(_data); m_data[h] = _data; return AssemblyItem(PushData, h); }
	AssemblyItem newPushString(std::string const& _data) { auto h = sha3(_data); m_strings[h] = _data; return AssemblyItem(PushString, h); }

	AssemblyItem append() { return append(newTag()); }
	void append(Assembly const& _a);
	void append(Assembly const& _a, int _deposit);
	AssemblyItem const& append(AssemblyItem const& _i);
	AssemblyItem const& append(std::string const& _data) { return append(newPushString(_data)); }
	AssemblyItem const& append(bytes const& _data) { return append(newData(_data)); }

	AssemblyItem appendJump() { auto ret = append(newPushTag()); append(Instruction::JUMP); return ret; }
	AssemblyItem appendJumpI() { auto ret = append(newPushTag()); append(Instruction::JUMPI); return ret; }
	AssemblyItem appendJump(AssemblyItem const& _tag) { auto ret = append(_tag.pushTag()); append(Instruction::JUMP); return ret; }
	AssemblyItem appendJumpI(AssemblyItem const& _tag) { auto ret = append(_tag.pushTag()); append(Instruction::JUMPI); return ret; }

	template <class T> Assembly& operator<<(T const& _d) { append(_d); return *this; }

	AssemblyItem const& back() { return m_items.back(); }
	std::string backString() const { return m_items.back().m_type == PushString ? m_strings.at((h256)m_items.back().m_data) : std::string(); }

	void onePath() { assert(!m_totalDeposit && !m_baseDeposit); m_baseDeposit = m_deposit; m_totalDeposit = INT_MAX; }
	void otherPath() { donePath(); m_totalDeposit = m_deposit; m_deposit = m_baseDeposit; }
	void donePaths() { donePath(); m_totalDeposit = m_baseDeposit = 0; }
	void ignored() { m_baseDeposit = m_deposit; }
	void endIgnored() { m_deposit = m_baseDeposit; m_baseDeposit = 0; }

	void popTo(int _deposit) { while (m_deposit > _deposit) append(Instruction::POP); }

	std::string out() const { std::stringstream ret; streamOut(ret); return ret.str(); }
	int deposit() const { return m_deposit; }
	bytes assemble() const;
	void optimise();
	std::ostream& streamOut(std::ostream& _out) const;

private:
	void donePath() { if (m_totalDeposit != INT_MAX && m_totalDeposit != m_deposit) throw InvalidDeposit(); }
	unsigned bytesRequired() const;

	unsigned m_usedTags = 0;
	AssemblyItems m_items;
	std::map<h256, bytes> m_data;
	std::map<h256, std::string> m_strings;

	int m_deposit = 0;
	int m_baseDeposit = 0;
	int m_totalDeposit = 0;
};

inline std::ostream& operator<<(std::ostream& _out, Assembly const& _a)
{
	_a.streamOut(_out);
	return _out;
}

}
