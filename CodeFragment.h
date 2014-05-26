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

namespace boost { namespace spirit { class utree; } }
namespace sp = boost::spirit;

namespace eth
{

class CompilerState;
class CodeFragment;

void debugOutAST(std::ostream& _out, sp::utree const& _this);

class CodeLocation
{
	friend class CodeFragment;

public:
	CodeLocation(CodeFragment* _f);
	CodeLocation(CodeFragment* _f, unsigned _p): m_f(_f), m_pos(_p) {}

	unsigned get() const;
	void increase(unsigned _val);
	void set(unsigned _val);
	void set(CodeLocation _loc) { assert(_loc.m_f == m_f); set(_loc.m_pos); }
	void anchor();

	CodeLocation operator+(unsigned _i) const { return CodeLocation(m_f, m_pos + _i); }

private:
	CodeFragment* m_f;
	unsigned m_pos;
};

class CompilerState;

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

class CodeFragment
{
	friend class CodeLocation;

public:
	CodeFragment(sp::utree const& _t, CompilerState& _s, bool _allowASM = false);
	CodeFragment(bytes const& _c = bytes()): m_code(_c) {}

	static CodeFragment compile(std::string const& _src, CompilerState& _s);

	/// Consolidates data and returns code.
	bytes const& code() { optimise(); consolidateData(); return m_code; }

	unsigned appendPush(u256 _l);
	void appendFragment(CodeFragment const& _f);
	void appendFragment(CodeFragment const& _f, unsigned _i);
	void appendInstruction(Instruction _i);

	CodeLocation appendPushLocation(unsigned _l = 0);
	void appendPushLocation(CodeLocation _l) { assert(_l.m_f == this); appendPushLocation(_l.m_pos); }
	void appendPushDataLocation(bytes const& _data);

	CodeLocation appendJump() { auto ret = appendPushLocation(0); appendInstruction(Instruction::JUMP); return ret; }
	CodeLocation appendJumpI() { auto ret = appendPushLocation(0); appendInstruction(Instruction::JUMPI); return ret; }
	CodeLocation appendJump(CodeLocation _l) { auto ret = appendPushLocation(_l.m_pos); appendInstruction(Instruction::JUMP); return ret; }
	CodeLocation appendJumpI(CodeLocation _l) { auto ret = appendPushLocation(_l.m_pos); appendInstruction(Instruction::JUMPI); return ret; }

	void appendFile(std::string const& _fn);

	std::string asPushedString() const;

	void onePath() { assert(!m_totalDeposit && !m_baseDeposit); m_baseDeposit = m_deposit; m_totalDeposit = INT_MAX; }
	void otherPath() { donePath(); m_totalDeposit = m_deposit; m_deposit = m_baseDeposit; }
	void donePaths() { donePath(); m_totalDeposit = m_baseDeposit = 0; }
	void ignored() { m_baseDeposit = m_deposit; }
	void endIgnored() { m_deposit = m_baseDeposit; m_baseDeposit = 0; }

	bool operator==(CodeFragment const& _f) const { return _f.m_code == m_code && _f.m_data == m_data; }
	bool operator!=(CodeFragment const& _f) const { return !operator==(_f); }
	unsigned size() const { return m_code.size(); }

	void consolidateData();
	void optimise();

private:
	template <class T> void error() const { throw T(); }
	void constructOperation(sp::utree const& _t, CompilerState& _s);

	void donePath() { if (m_totalDeposit != INT_MAX && m_totalDeposit != m_deposit) error<InvalidDeposit>(); }

	int m_deposit = 0;
	int m_baseDeposit = 0;
	int m_totalDeposit = 0;
	bytes m_code;
	std::vector<unsigned> m_locs;
	std::multimap<bytes, unsigned> m_data;
};

static const CodeFragment NullCodeFragment;

}
