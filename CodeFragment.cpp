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
/** @file CodeFragment.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "Parser.h"
#include "CodeFragment.h"

#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/support_utree.hpp>
#include <libethsupport/Log.h>
#include <libethcore/Instruction.h>
#include <libethcore/CommonEth.h>
#include "CompilerState.h"
using namespace std;
using namespace eth;
namespace qi = boost::spirit::qi;
namespace px = boost::phoenix;
namespace sp = boost::spirit;

void eth::debugOutAST(ostream& _out, sp::utree const& _this)
{
	switch (_this.which())
	{
	case sp::utree_type::list_type:
		switch (_this.tag())
		{
		case 0: _out << "( "; for (auto const& i: _this) { debugOutAST(_out, i); _out << " "; } _out << ")"; break;
		case 1: _out << "@ "; debugOutAST(_out, _this.front()); break;
		case 2: _out << "@@ "; debugOutAST(_out, _this.front()); break;
		case 3: _out << "[ "; debugOutAST(_out, _this.front()); _out << " ] "; debugOutAST(_out, _this.back()); break;
		case 4: _out << "[[ "; debugOutAST(_out, _this.front()); _out << " ]] "; debugOutAST(_out, _this.back()); break;
		case 5: _out << "{ "; for (auto const& i: _this) { debugOutAST(_out, i); _out << " "; } _out << "}"; break;
		default:;
		}

		break;
	case sp::utree_type::int_type: _out << _this.get<int>(); break;
	case sp::utree_type::string_type: _out << "\"" << _this.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::string_type>>() << "\""; break;
	case sp::utree_type::symbol_type: _out << _this.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::symbol_type>>(); break;
	case sp::utree_type::any_type: _out << *_this.get<bigint*>(); break;
	default: _out << "nil";
	}
}

void CodeFragment::appendFragment(CodeFragment const& _f)
{
	m_locs.reserve(m_locs.size() + _f.m_locs.size());
	m_code.reserve(m_code.size() + _f.m_code.size());

	unsigned os = m_code.size();

	for (auto i: _f.m_code)
		m_code.push_back(i);

	for (auto i: _f.m_locs)
	{
		CodeLocation(this, i + os).increase(os);
		m_locs.push_back(i + os);
	}

	for (auto i: _f.m_data)
		m_data.insert(make_pair(i.first, i.second + os));

	m_deposit += _f.m_deposit;
}

CodeFragment CodeFragment::compile(string const& _src, CompilerState& _s)
{
	CodeFragment ret;
	sp::utree o;
	parseTreeLLL(_src, o);
	if (!o.empty())
		ret = CodeFragment(o, _s);
	_s.treesToKill.push_back(o);
	return ret;
}

void CodeFragment::consolidateData()
{
	m_code.push_back(0);
	bytes ld;
	for (auto const& i: m_data)
	{
		if (ld != i.first)
		{
			ld = i.first;
			for (auto j: ld)
				m_code.push_back(j);
		}
		CodeLocation(this, i.second).set(m_code.size() - ld.size());
	}
	m_data.clear();
}

void CodeFragment::appendFragment(CodeFragment const& _f, unsigned _deposit)
{
	if ((int)_deposit > _f.m_deposit)
		error<InvalidDeposit>();
	else
	{
		appendFragment(_f);
		while (_deposit++ < (unsigned)_f.m_deposit)
			appendInstruction(Instruction::POP);
	}
}

CodeLocation CodeFragment::appendPushLocation(unsigned _locationValue)
{
	m_code.push_back((byte)Instruction::PUSH4);
	CodeLocation ret(this, m_code.size());
	m_locs.push_back(m_code.size());
	m_code.resize(m_code.size() + 4);
	bytesRef r(&m_code[m_code.size() - 4], 4);
	toBigEndian(_locationValue, r);
	m_deposit++;
	return ret;
}

unsigned CodeFragment::appendPush(u256 _literalValue)
{
	unsigned br = max<unsigned>(1, bytesRequired(_literalValue));
	m_code.push_back((byte)Instruction::PUSH1 + br - 1);
	m_code.resize(m_code.size() + br);
	for (unsigned i = 0; i < br; ++i)
	{
		m_code[m_code.size() - 1 - i] = (byte)(_literalValue & 0xff);
		_literalValue >>= 8;
	}
	m_deposit++;
	return br + 1;
}

void CodeFragment::appendInstruction(Instruction _i)
{
	m_code.push_back((byte)_i);
	m_deposit += c_instructionInfo.at(_i).ret - c_instructionInfo.at(_i).args;
}

CodeFragment::CodeFragment(sp::utree const& _t, CompilerState& _s, bool _allowASM)
{
/*	cdebug << "CodeFragment. Locals:";
	for (auto const& i: _s.defs)
		cdebug << i.first << ":" << toHex(i.second.m_code);
	cdebug << "Args:";
	for (auto const& i: _s.args)
		cdebug << i.first << ":" << toHex(i.second.m_code);
	cdebug << "Outers:";
	for (auto const& i: _s.outers)
		cdebug << i.first << ":" << toHex(i.second.m_code);
	debugOutAST(cout, _t);
	cout << endl << flush;
*/
	switch (_t.which())
	{
	case sp::utree_type::list_type:
		constructOperation(_t, _s);
		break;
	case sp::utree_type::string_type:
	{
		auto sr = _t.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::string_type>>();
		string s(sr.begin(), sr.end());
		if (s.size() > 32)
			error<StringTooLong>();
		h256 valHash;
		memcpy(valHash.data(), s.data(), s.size());
		memset(valHash.data() + s.size(), 0, 32 - s.size());
		appendPush(valHash);
		break;
	}
	case sp::utree_type::symbol_type:
	{
		auto sr = _t.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::symbol_type>>();
		string s(sr.begin(), sr.end());
		string us = boost::algorithm::to_upper_copy(s);
		if (_allowASM)
		{
			if (c_instructions.count(us))
			{
				auto it = c_instructions.find(us);
				m_deposit = c_instructionInfo.at(it->second).ret - c_instructionInfo.at(it->second).args;
				m_code.push_back((byte)it->second);
			}
		}
		if (_s.defs.count(s))
			appendFragment(_s.defs.at(s));
		else if (_s.args.count(s))
			appendFragment(_s.args.at(s));
		else if (_s.outers.count(s))
			appendFragment(_s.outers.at(s));
		else if (us.find_first_of("1234567890") != 0 && us.find_first_not_of("QWERTYUIOPASDFGHJKLZXCVBNM1234567890_") == string::npos)
		{
			auto it = _s.vars.find(s);
			if (it == _s.vars.end())
			{
				bool ok;
				tie(it, ok) = _s.vars.insert(make_pair(s, _s.vars.size() * 32));
			}
			appendPush(it->second);
		}
		else
			error<BareSymbol>();

		break;
	}
	case sp::utree_type::any_type:
	{
		bigint i = *_t.get<bigint*>();
		if (i < 0 || i > bigint(u256(0) - 1))
			error<IntegerOutOfRange>();
		appendPush((u256)i);
		break;
	}
	default: break;
	}
}

void CodeFragment::appendPushDataLocation(bytes const& _data)
{
	m_code.push_back((byte)Instruction::PUSH4);
	m_data.insert(make_pair(_data, m_code.size()));
	m_code.resize(m_code.size() + 4);
	memset(&m_code.back() - 3, 0, 4);
	m_deposit++;
}

std::string CodeFragment::asPushedString() const
{
	string ret;
	if (m_code.size())
	{
		unsigned bc = m_code[0] - (byte)Instruction::PUSH1 + 1;
		if (m_code[0] >= (byte)Instruction::PUSH1 && m_code[0] <= (byte)Instruction::PUSH32)
		{
			for (unsigned s = 0; s < bc && m_code[1 + s]; ++s)
				ret.push_back(m_code[1 + s]);
			return ret;
		}
	}
	error<ExpectedLiteral>();
	return ret;
}

void CodeFragment::optimise()
{
//	map<string, function<bytes(vector<u256> const&)>> pattern = { { "PUSH,PUSH,ADD", [](vector<u256> const& v) { return CodeFragment(appendPush(v[0] + v[1])); } } };
}

void CodeFragment::constructOperation(sp::utree const& _t, CompilerState& _s)
{
	if (_t.empty())
		error<EmptyList>();
	else if (_t.tag() == 0 && _t.front().which() != sp::utree_type::symbol_type)
		error<DataNotExecutable>();
	else
	{
		string s;
		string us;
		switch (_t.tag())
		{
		case 0:
		{
			auto sr = _t.front().get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::symbol_type>>();
			s = string(sr.begin(), sr.end());
			us = boost::algorithm::to_upper_copy(s);
			break;
		}
		case 1:
			us = "MLOAD";
			break;
		case 2:
			us = "SLOAD";
			break;
		case 3:
			us = "MSTORE";
			break;
		case 4:
			us = "SSTORE";
			break;
		case 5:
			us = "SEQ";
			break;
		default:;
		}

		// Operations who args are not standard stack-pushers.
		bool nonStandard = true;
		if (us == "ASM")
		{
			int c = 0;
			for (auto const& i: _t)
				if (c++)
					appendFragment(CodeFragment(i, _s, true));
		}
		else if (us == "INCLUDE")
		{
			if (_t.size() != 2)
				error<IncorrectParameterCount>();
			string n;
			auto i = *++_t.begin();
			if (i.tag())
				error<InvalidName>();
			if (i.which() == sp::utree_type::string_type)
			{
				auto sr = i.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::string_type>>();
				n = string(sr.begin(), sr.end());
			}
			else if (i.which() == sp::utree_type::symbol_type)
			{
				auto sr = i.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::symbol_type>>();
				n = _s.getDef(string(sr.begin(), sr.end())).asPushedString();
			}
			appendFragment(CodeFragment::compile(asString(contents(n)), _s));
		}
		else if (us == "DEF")
		{
			string n;
			unsigned ii = 0;
			if (_t.size() != 3 && _t.size() != 4)
				error<IncorrectParameterCount>();
			for (auto const& i: _t)
			{
				if (ii == 1)
				{
					if (i.tag())
						error<InvalidName>();
					if (i.which() == sp::utree_type::string_type)
					{
						auto sr = i.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::string_type>>();
						n = string(sr.begin(), sr.end());
					}
					else if (i.which() == sp::utree_type::symbol_type)
					{
						auto sr = i.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::symbol_type>>();
						n = _s.getDef(string(sr.begin(), sr.end())).asPushedString();
					}
				}
				else if (ii == 2)
					if (_t.size() == 3)
						_s.defs[n] = CodeFragment(i, _s);
					else
						for (auto const& j: i)
						{
							if (j.tag() || j.which() != sp::utree_type::symbol_type)
								error<InvalidMacroArgs>();
							auto sr = j.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::symbol_type>>();
							_s.macros[n].args.push_back(string(sr.begin(), sr.end()));
						}
				else if (ii == 3)
				{
					_s.macros[n].code = i;
					_s.macros[n].env = _s.outers;
					for (auto const& i: _s.args)
						_s.macros[n].env[i.first] = i.second;
					for (auto const& i: _s.defs)
						_s.macros[n].env[i.first] = i.second;
				}
				++ii;
			}

		}
		else if (us == "LIT")
		{
			if (_t.size() < 3)
				error<IncorrectParameterCount>();
			unsigned ii = 0;
			CodeFragment pos;
			bytes data;
			for (auto const& i: _t)
			{
				if (ii == 1)
				{
					pos = CodeFragment(i, _s);
					if (pos.m_deposit != 1)
						error<InvalidDeposit>();
				}
				else if (ii == 2 && !i.tag() && i.which() == sp::utree_type::string_type)
				{
					auto sr = i.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::string_type>>();
					data = bytes((byte const*)sr.begin(), (byte const*)sr.end());
				}
				else if (ii >= 2 && !i.tag() && i.which() == sp::utree_type::any_type)
				{
					bigint bi = *i.get<bigint*>();
					if (bi < 0)
						error<IntegerOutOfRange>();
					else if (bi > bigint(u256(0) - 1))
					{
						if (ii == 2 && _t.size() == 3)
						{
							// One big int - allow it as hex.
							data.resize(bytesRequired(bi));
							toBigEndian(bi, data);
						}
						else
							error<IntegerOutOfRange>();
					}
					else
					{
						data.resize(data.size() + 32);
						*(h256*)(&data.back() - 31) = (u256)bi;
					}
				}
				else if (ii)
					error<InvalidLiteral>();
				++ii;
			}
			appendPush(data.size());
			appendInstruction(Instruction::DUP);
			appendPushDataLocation(data);
			appendFragment(pos, 1);
			appendInstruction(Instruction::CODECOPY);
		}
		else
			nonStandard = false;

		if (nonStandard)
			return;

		std::map<std::string, Instruction> const c_arith = { { "+", Instruction::ADD }, { "-", Instruction::SUB }, { "*", Instruction::MUL }, { "/", Instruction::DIV }, { "%", Instruction::MOD }, { "&", Instruction::AND }, { "|", Instruction::OR }, { "^", Instruction::XOR } };
		std::map<std::string, pair<Instruction, bool>> const c_binary = { { "<", { Instruction::LT, false } }, { "<=", { Instruction::GT, true } }, { ">", { Instruction::GT, false } }, { ">=", { Instruction::LT, true } }, { "S<", { Instruction::SLT, false } }, { "S<=", { Instruction::SGT, true } }, { "S>", { Instruction::SGT, false } }, { "S>=", { Instruction::SLT, true } }, { "=", { Instruction::EQ, false } }, { "!=", { Instruction::EQ, true } } };
		std::map<std::string, Instruction> const c_unary = { { "!", Instruction::NOT } };

		vector<CodeFragment> code;
		CompilerState ns = _s;
		ns.vars.clear();
		int c = _t.tag() ? 1 : 0;
		for (auto const& i: _t)
			if (c++)
			{
				if (us == "LLL" && c == 1)
					code.push_back(CodeFragment(i, ns));
				else
					code.push_back(CodeFragment(i, _s));
			}
		auto requireSize = [&](unsigned s) { if (code.size() != s) error<IncorrectParameterCount>(); };
		auto requireMinSize = [&](unsigned s) { if (code.size() < s) error<IncorrectParameterCount>(); };
		auto requireMaxSize = [&](unsigned s) { if (code.size() > s) error<IncorrectParameterCount>(); };
		auto requireDeposit = [&](unsigned i, int s) { if (code[i].m_deposit != s) error<InvalidDeposit>(); };

		if (_s.macros.count(s) && _s.macros.at(s).args.size() == code.size())
		{
			Macro const& m = _s.macros.at(s);
			CompilerState cs = _s;
			for (auto const& i: m.env)
				cs.outers[i.first] = i.second;
			for (auto const& i: cs.defs)
				cs.outers[i.first] = i.second;
			cs.defs.clear();
			for (unsigned i = 0; i < m.args.size(); ++i)
			{
				requireDeposit(i, 1);
				cs.args[m.args[i]] = code[i];
			}
			appendFragment(CodeFragment(m.code, cs));
			for (auto const& i: cs.defs)
				_s.defs[i.first] = i.second;
			for (auto const& i: cs.macros)
				_s.macros.insert(i);
		}
		else if (c_instructions.count(us))
		{
			auto it = c_instructions.find(us);
			int ea = c_instructionInfo.at(it->second).args;
			if (ea >= 0)
				requireSize(ea);
			else
				requireMinSize(-ea);

			for (unsigned i = code.size(); i; --i)
				appendFragment(code[i - 1], 1);
			appendInstruction(it->second);
		}
		else if (c_arith.count(us))
		{
			auto it = c_arith.find(us);
			requireMinSize(1);
			for (unsigned i = code.size(); i; --i)
			{
				requireDeposit(i - 1, 1);
				appendFragment(code[i - 1], 1);
			}
			for (unsigned i = 1; i < code.size(); ++i)
				appendInstruction(it->second);
		}
		else if (c_binary.count(us))
		{
			auto it = c_binary.find(us);
			requireSize(2);
			requireDeposit(0, 1);
			requireDeposit(1, 1);
			appendFragment(code[1], 1);
			appendFragment(code[0], 1);
			appendInstruction(it->second.first);
			if (it->second.second)
				appendInstruction(Instruction::NOT);
		}
		else if (c_unary.count(us))
		{
			auto it = c_unary.find(us);
			requireSize(1);
			requireDeposit(0, 1);
			appendFragment(code[0], 1);
			appendInstruction(it->second);
		}
		else if (us == "IF")
		{
			requireSize(3);
			requireDeposit(0, 1);
			appendFragment(code[0]);
			auto pos = appendJumpI();
			onePath();
			appendFragment(code[2]);
			auto end = appendJump();
			otherPath();
			pos.anchor();
			appendFragment(code[1]);
			donePaths();
			end.anchor();
		}
		else if (us == "WHEN" || us == "UNLESS")
		{
			requireSize(2);
			requireDeposit(0, 1);
			appendFragment(code[0]);
			if (us == "WHEN")
				appendInstruction(Instruction::NOT);
			auto end = appendJumpI();
			onePath();
			otherPath();
			appendFragment(code[1], 0);
			donePaths();
			end.anchor();
		}
		else if (us == "WHILE")
		{
			requireSize(2);
			requireDeposit(0, 1);
			auto begin = CodeLocation(this);
			appendFragment(code[0], 1);
			appendInstruction(Instruction::NOT);
			auto end = appendJumpI();
			appendFragment(code[1], 0);
			appendJump(begin);
			end.anchor();
		}
		else if (us == "FOR")
		{
			requireSize(4);
			requireDeposit(1, 1);
			appendFragment(code[0], 0);
			auto begin = CodeLocation(this);
			appendFragment(code[1], 1);
			appendInstruction(Instruction::NOT);
			auto end = appendJumpI();
			appendFragment(code[3], 0);
			appendFragment(code[2], 0);
			appendJump(begin);
			end.anchor();
		}
		else if (us == "LLL")
		{
			requireMinSize(2);
			requireMaxSize(3);
			requireDeposit(1, 1);

			CodeLocation codeloc(this, m_code.size() + 6);
			bytes const& subcode = code[0].code();
			appendPush(subcode.size());
			appendInstruction(Instruction::DUP);
			if (code.size() == 3)
			{
				requireDeposit(2, 1);
				appendFragment(code[2], 1);
				appendInstruction(Instruction::LT);
				appendInstruction(Instruction::NOT);
				appendInstruction(Instruction::MUL);
				appendInstruction(Instruction::DUP);
			}
			appendPushDataLocation(subcode);
			appendFragment(code[1], 1);
			appendInstruction(Instruction::CODECOPY);
		}
		else if (us == "&&" || us == "||")
		{
			requireMinSize(1);
			for (unsigned i = 0; i < code.size(); ++i)
				requireDeposit(i, 1);

			vector<CodeLocation> ends;
			if (code.size() > 1)
			{
				appendPush(us == "||" ? 1 : 0);
				for (unsigned i = 1; i < code.size(); ++i)
				{
					// Check if true - predicate
					appendFragment(code[i - 1], 1);
					if (us == "&&")
						appendInstruction(Instruction::NOT);
					ends.push_back(appendJumpI());
				}
				appendInstruction(Instruction::POP);
			}

			// Check if true - predicate
			appendFragment(code.back(), 1);

			// At end now.
			for (auto& i: ends)
				i.anchor();
		}
		else if (us == "~")
		{
			requireSize(1);
			requireDeposit(0, 1);
			appendFragment(code[0], 1);
			appendPush(1);
			appendPush(0);
			appendInstruction(Instruction::SUB);
			appendInstruction(Instruction::SUB);
		}
		else if (us == "SEQ")
		{
			unsigned ii = 0;
			for (auto const& i: code)
				if (++ii < code.size())
					appendFragment(i, 0);
				else
					appendFragment(i);
		}
		else if (us == "RAW")
		{
			for (auto const& i: code)
				appendFragment(i);
			while (m_deposit > 1)
				appendInstruction(Instruction::POP);
		}
		else if (us.find_first_of("1234567890") != 0 && us.find_first_not_of("QWERTYUIOPASDFGHJKLZXCVBNM1234567890_") == string::npos)
		{
			auto it = _s.vars.find(s);
			if (it == _s.vars.end())
			{
				bool ok;
				tie(it, ok) = _s.vars.insert(make_pair(s, _s.vars.size() * 32));
			}
			appendPush(it->second);
		}
		else
			error<InvalidOperation>();
	}
}
