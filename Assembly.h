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

enum AssemblyItemType { UndefinedItem, Operation, Push, PushString, PushTag, PushSub, PushSubSize, PushProgramSize, Tag, PushData, NoOptimizeBegin, NoOptimizeEnd };

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
	u256 data() const { return m_data; }

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

std::ostream& operator<<(std::ostream& _out, AssemblyItemsConstRef _i);
inline std::ostream& operator<<(std::ostream& _out, AssemblyItems const& _i) { return operator<<(_out, AssemblyItemsConstRef(&_i)); }

class Assembly
{
public:
	Assembly() {}

	AssemblyItem newTag() { return AssemblyItem(Tag, m_usedTags++); }
	AssemblyItem newPushTag() { return AssemblyItem(PushTag, m_usedTags++); }
	AssemblyItem newData(bytes const& _data) { h256 h = (u256)std::hash<std::string>()(asString(_data)); m_data[h] = _data; return AssemblyItem(PushData, h); }
	AssemblyItem newSub(Assembly const& _sub) { m_subs.push_back(_sub); return AssemblyItem(PushSub, m_subs.size() - 1); }
	AssemblyItem newPushString(std::string const& _data) { h256 h = (u256)std::hash<std::string>()(_data); m_strings[h] = _data; return AssemblyItem(PushString, h); }
	AssemblyItem newPushSubSize(u256 const& _subId) { return AssemblyItem(PushSubSize, _subId); }

	AssemblyItem append() { return append(newTag()); }
	void append(Assembly const& _a);
	void append(Assembly const& _a, int _deposit);
	AssemblyItem const& append(AssemblyItem const& _i);
	AssemblyItem const& append(std::string const& _data) { return append(newPushString(_data)); }
	AssemblyItem const& append(bytes const& _data) { return append(newData(_data)); }
	AssemblyItem appendSubSize(Assembly const& _a) { auto ret = newSub(_a); append(newPushSubSize(ret.data())); return ret; }
	/// Pushes the final size of the current assembly itself. Use this when the code is modified
	/// after compilation and CODESIZE is not an option.
	void appendProgramSize() { append(AssemblyItem(PushProgramSize)); }

	AssemblyItem appendJump() { auto ret = append(newPushTag()); append(Instruction::JUMP); return ret; }
	AssemblyItem appendJumpI() { auto ret = append(newPushTag()); append(Instruction::JUMPI); return ret; }
	AssemblyItem appendJump(AssemblyItem const& _tag) { auto ret = append(_tag.pushTag()); append(Instruction::JUMP); return ret; }
	AssemblyItem appendJumpI(AssemblyItem const& _tag) { auto ret = append(_tag.pushTag()); append(Instruction::JUMPI); return ret; }
	template <class T> Assembly& operator<<(T const& _d) { append(_d); return *this; }
	AssemblyItems const& getItems() const { return m_items; }
	AssemblyItem const& back() const { return m_items.back(); }
	std::string backString() const { return m_items.size() && m_items.back().m_type == PushString ? m_strings.at((h256)m_items.back().m_data) : std::string(); }

	void onePath() { if (asserts(!m_totalDeposit && !m_baseDeposit)) BOOST_THROW_EXCEPTION(InvalidDeposit()); m_baseDeposit = m_deposit; m_totalDeposit = INT_MAX; }
	void otherPath() { donePath(); m_totalDeposit = m_deposit; m_deposit = m_baseDeposit; }
	void donePaths() { donePath(); m_totalDeposit = m_baseDeposit = 0; }
	void ignored() { m_baseDeposit = m_deposit; }
	void endIgnored() { m_deposit = m_baseDeposit; m_baseDeposit = 0; }

	void popTo(int _deposit) { while (m_deposit > _deposit) append(Instruction::POP); }

	void injectStart(AssemblyItem const& _i);
	std::string out() const { std::stringstream ret; stream(ret); return ret.str(); }
	int deposit() const { return m_deposit; }
	void adjustDeposit(int _adjustment) { m_deposit += _adjustment; if (asserts(m_deposit >= 0)) BOOST_THROW_EXCEPTION(InvalidDeposit()); }
	void setDeposit(int _deposit) { m_deposit = _deposit; if (asserts(m_deposit >= 0)) BOOST_THROW_EXCEPTION(InvalidDeposit()); }

	/// Changes the source location used for each appended item.
	void setSourceLocation(SourceLocation const& _location) { m_currentSourceLocation = _location; }

	bytes assemble() const;
	Assembly& optimise(bool _enable);
	std::ostream& stream(std::ostream& _out, std::string const& _prefix = "", const StringMap &_sourceCodes = StringMap()) const;

protected:
	std::string getLocationFromSources(StringMap const& _sourceCodes, SourceLocation const& _location) const;
	void donePath() { if (m_totalDeposit != INT_MAX && m_totalDeposit != m_deposit) BOOST_THROW_EXCEPTION(InvalidDeposit()); }
	unsigned bytesRequired() const;

	unsigned m_usedTags = 0;
	AssemblyItems m_items;
	mutable std::map<h256, bytes> m_data;
	std::vector<Assembly> m_subs;
	std::map<h256, std::string> m_strings;

	int m_deposit = 0;
	int m_baseDeposit = 0;
	int m_totalDeposit = 0;

	SourceLocation m_currentSourceLocation;
};

inline std::ostream& operator<<(std::ostream& _out, Assembly const& _a)
{
	_a.stream(_out);
	return _out;
}

}
}
