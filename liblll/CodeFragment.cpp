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
/** @file CodeFragment.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include <liblll/CodeFragment.h>
#include <liblll/CompilerState.h>
#include <liblll/Parser.h>
#include <libevmasm/Instruction.h>
#include <libdevcore/CommonIO.h>

#include <boost/algorithm/string.hpp>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // defined(__GNUC__)

#include <boost/spirit/include/support_utree.hpp>

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif // defined(__GNUC__)


using namespace std;
using namespace dev;
using namespace dev::lll;

void CodeFragment::finalise(CompilerState const& _cs)
{
	// NOTE: add this as a safeguard in case the user didn't issue an
	// explicit stop at the end of the sequence
	m_asm.append(Instruction::STOP);

	if (_cs.usedAlloc && _cs.vars.size() && !m_finalised)
	{
		m_finalised = true;
		m_asm.injectStart(Instruction::MSTORE8);
		m_asm.injectStart((u256)((_cs.vars.size() + 2) * 32) - 1);
		m_asm.injectStart((u256)1);
	}
}

namespace
{
/// Returns true iff the instruction is valid in "inline assembly".
bool validAssemblyInstruction(string us)
{
	auto it = c_instructions.find(us);
	return !(
		it == c_instructions.end() ||
		solidity::isPushInstruction(it->second)
	);
}

/// Returns true iff the instruction is valid as a function.
bool validFunctionalInstruction(string us)
{
	auto it = c_instructions.find(us);
	return !(
		it == c_instructions.end() ||
		solidity::isPushInstruction(it->second) ||
		solidity::isDupInstruction(it->second) ||
		solidity::isSwapInstruction(it->second) ||
		it->second == solidity::Instruction::JUMPDEST
	);
}
}

CodeFragment::CodeFragment(sp::utree const& _t, CompilerState& _s, ReadCallback const& _readFile, bool _allowASM):
	m_readFile(_readFile)
{
/*
	std::cout << "CodeFragment. Locals:";
	for (auto const& i: _s.defs)
		std::cout << i.first << ":" << i.second.m_asm.out();
	std::cout << "Args:";
	for (auto const& i: _s.args)
		std::cout << i.first << ":" << i.second.m_asm.out();
	std::cout << "Outers:";
	for (auto const& i: _s.outers)
		std::cout << i.first << ":" << i.second.m_asm.out();
	debugOutAST(std::cout, _t);
	std::cout << endl << flush;
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
		m_asm.append(s);
		break;
	}
	case sp::utree_type::symbol_type:
	{
		auto sr = _t.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::symbol_type>>();
		string s(sr.begin(), sr.end());
		string us = boost::algorithm::to_upper_copy(s);
		if (_allowASM && c_instructions.count(us) && validAssemblyInstruction(us))
			m_asm.append(c_instructions.at(us));
		else if (_s.defs.count(s))
			m_asm.append(_s.defs.at(s).m_asm);
		else if (_s.args.count(s))
			m_asm.append(_s.args.at(s).m_asm);
		else if (_s.outers.count(s))
			m_asm.append(_s.outers.at(s).m_asm);
		else if (us.find_first_of("1234567890") != 0 && us.find_first_not_of("QWERTYUIOPASDFGHJKLZXCVBNM1234567890_-") == string::npos)
		{
			auto it = _s.vars.find(s);
			if (it == _s.vars.end())
				error<InvalidName>(std::string("Symbol not found: ") + s);
			m_asm.append((u256)it->second.first);
		}
		else
			error<BareSymbol>(s);

		break;
	}
	case sp::utree_type::any_type:
	{
		bigint i = *_t.get<bigint*>();
		if (i < 0 || i > bigint(u256(0) - 1))
			error<IntegerOutOfRange>(toString(i));
		m_asm.append((u256)i);
		break;
	}
	default:
		error<CompilerException>("Unexpected fragment type");
		break;
	}
}

void CodeFragment::constructOperation(sp::utree const& _t, CompilerState& _s)
{
	if (_t.tag() == 0 && _t.empty())
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
		case 6:
			us = "CALLDATALOAD";
			break;
		default:;
		}

		auto firstAsString = [&]()
		{
			auto i = *++_t.begin();
			if (i.tag())
				error<InvalidName>(toString(i));
			if (i.which() == sp::utree_type::string_type)
			{
				auto sr = i.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::string_type>>();
				return string(sr.begin(), sr.end());
			}
			else if (i.which() == sp::utree_type::symbol_type)
			{
				auto sr = i.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::symbol_type>>();
				return _s.getDef(string(sr.begin(), sr.end())).m_asm.backString();
			}
			return string();
		};

		auto varAddress = [&](string const& n, bool createMissing = false)
		{
			if (n.empty())
				error<InvalidName>("Empty variable name not allowed");
			auto it = _s.vars.find(n);
			if (it == _s.vars.end())
			{
				if (createMissing)
				{
					// Create new variable
					bool ok;
					tie(it, ok) = _s.vars.insert(make_pair(n, make_pair(_s.stackSize, 32)));
					_s.stackSize += 32;
				}
				else
					error<InvalidName>(std::string("Symbol not found: ") + n);
			}
			return it->second.first;
		};

		// Operations who args are not standard stack-pushers.
		bool nonStandard = true;
		if (us == "ASM")
		{
			int c = 0;
			for (auto const& i: _t)
				if (c++)
				{
					auto fragment = CodeFragment(i, _s, m_readFile, true).m_asm;
					if ((m_asm.deposit() + fragment.deposit()) < 0)
						error<IncorrectParameterCount>("The assembly instruction resulted in stack underflow");
					m_asm.append(fragment);
				}
		}
		else if (us == "INCLUDE")
		{
			if (_t.size() != 2)
				error<IncorrectParameterCount>(us);
			string fileName = firstAsString();
			if (fileName.empty())
				error<InvalidName>("Empty file name provided");
			if (!m_readFile)
				error<InvalidName>("Import callback not present");
			string contents = m_readFile(fileName);
			if (contents.empty())
				error<InvalidName>(std::string("File not found (or empty): ") + fileName);
			m_asm.append(CodeFragment::compile(contents, _s, m_readFile).m_asm);
		}
		else if (us == "SET")
		{
			// TODO: move this to be a stack variable (and not a memory variable)
			if (_t.size() != 3)
				error<IncorrectParameterCount>(us);
			int c = 0;
			for (auto const& i: _t)
				if (c++ == 2)
					m_asm.append(CodeFragment(i, _s, m_readFile, false).m_asm);
			m_asm.append((u256)varAddress(firstAsString(), true));
			m_asm.append(Instruction::MSTORE);
		}
		else if (us == "UNSET")
		{
			// TODO: this doesn't actually free up anything, since it is a memory variable (see "SET")
			if (_t.size() != 2)
				error<IncorrectParameterCount>();
			auto it = _s.vars.find(firstAsString());
			if (it != _s.vars.end())
				_s.vars.erase(it);
		}
		else if (us == "GET")
		{
			if (_t.size() != 2)
				error<IncorrectParameterCount>(us);
			m_asm.append((u256)varAddress(firstAsString()));
			m_asm.append(Instruction::MLOAD);
		}
		else if (us == "WITH")
		{
			if (_t.size() != 4)
				error<IncorrectParameterCount>();
			string key = firstAsString();
			if (_s.vars.find(key) != _s.vars.end())
				error<InvalidName>(string("Symbol already used: ") + key);

			// Create variable
			// TODO: move this to be a stack variable (and not a memory variable)
			size_t c = 0;
			for (auto const& i: _t)
				if (c++ == 2)
					m_asm.append(CodeFragment(i, _s, m_readFile, false).m_asm);
			m_asm.append((u256)varAddress(key, true));
			m_asm.append(Instruction::MSTORE);

			// Insert sub with variable access, but new state
			CompilerState ns = _s;
			c = 0;
			for (auto const& i: _t)
				if (c++ == 3)
					m_asm.append(CodeFragment(i, _s, m_readFile, false).m_asm);

			// Remove variable
			auto it = _s.vars.find(key);
			if (it != _s.vars.end())
				_s.vars.erase(it);
		}
		else if (us == "REF")
			m_asm.append((u256)varAddress(firstAsString()));
		else if (us == "DEF")
		{
			string n;
			unsigned ii = 0;
			if (_t.size() != 3 && _t.size() != 4)
				error<IncorrectParameterCount>(us);
			vector<string> args;
			for (auto const& i: _t)
			{
				if (ii == 1)
				{
					if (i.tag())
						error<InvalidName>(toString(i));
					if (i.which() == sp::utree_type::string_type)
					{
						auto sr = i.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::string_type>>();
						n = string(sr.begin(), sr.end());
					}
					else if (i.which() == sp::utree_type::symbol_type)
					{
						auto sr = i.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::symbol_type>>();
						n = _s.getDef(string(sr.begin(), sr.end())).m_asm.backString();
					}
				}
				else if (ii == 2)
					if (_t.size() == 3)
					{
						/// NOTE: some compilers could do the assignment first if this is done in a single line
						CodeFragment code = CodeFragment(i, _s, m_readFile);
						_s.defs[n] = code;
					}
					else
						for (auto const& j: i)
						{
							if (j.tag() || j.which() != sp::utree_type::symbol_type)
								error<InvalidMacroArgs>();
							auto sr = j.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::symbol_type>>();
							args.emplace_back(sr.begin(), sr.end());
						}
				else if (ii == 3)
				{
					auto k = make_pair(n, args.size());
					_s.macros[k].code = i;
					_s.macros[k].env = _s.outers;
					_s.macros[k].args = args;
					for (auto const& i: _s.args)
						_s.macros[k].env[i.first] = i.second;
					for (auto const& i: _s.defs)
						_s.macros[k].env[i.first] = i.second;
				}
				++ii;
			}
		}
		else if (us == "LIT")
		{
			if (_t.size() < 3)
				error<IncorrectParameterCount>(us);
			unsigned ii = 0;
			CodeFragment pos;
			bytes data;
			for (auto const& i: _t)
			{
				if (ii == 0)
				{
					ii++;
					continue;
				}
				else if (ii == 1)
				{
					pos = CodeFragment(i, _s, m_readFile);
					if (pos.m_asm.deposit() != 1)
						error<InvalidDeposit>(toString(i));
				}
				else if (i.tag() != 0)
				{
					error<InvalidLiteral>(toString(i));
				}
				else if (i.which() == sp::utree_type::string_type)
				{
					auto sr = i.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::string_type>>();
					data.insert(data.end(), (uint8_t const *)sr.begin(), (uint8_t const*)sr.end());
				}
				else if (i.which() == sp::utree_type::any_type)
				{
					bigint bi = *i.get<bigint*>();
					if (bi < 0)
						error<IntegerOutOfRange>(toString(i));
					else
					{
						bytes tmp = toCompactBigEndian(bi);
						data.insert(data.end(), tmp.begin(), tmp.end());
					}
				}
				else
				{
					error<InvalidLiteral>(toString(i));
				}

				ii++;
			}
			m_asm.append((u256)data.size());
			m_asm.append(Instruction::DUP1);
			m_asm.append(data);
			m_asm.append(pos.m_asm, 1);
			m_asm.append(Instruction::CODECOPY);
		}
		else
			nonStandard = false;

		if (nonStandard)
			return;

		std::map<std::string, Instruction> const c_arith = {
			{ "+", Instruction::ADD },
			{ "-", Instruction::SUB },
			{ "*", Instruction::MUL },
			{ "/", Instruction::DIV },
			{ "%", Instruction::MOD },
			{ "&", Instruction::AND },
			{ "|", Instruction::OR },
			{ "^", Instruction::XOR }
		};
		std::map<std::string, pair<Instruction, bool>> const c_binary = {
			{ "<", { Instruction::LT, false } },
			{ "<=", { Instruction::GT, true } },
			{ ">", { Instruction::GT, false } },
			{ ">=", { Instruction::LT, true } },
			{ "S<", { Instruction::SLT, false } },
			{ "S<=", { Instruction::SGT, true } },
			{ "S>", { Instruction::SGT, false } },
			{ "S>=", { Instruction::SLT, true } },
			{ "=", { Instruction::EQ, false } },
			{ "!=", { Instruction::EQ, true } }
		};
		std::map<std::string, Instruction> const c_unary = {
			{ "!", Instruction::ISZERO },
			{ "~", Instruction::NOT }
		};

		vector<CodeFragment> code;
		CompilerState ns = _s;
		ns.vars.clear();
		ns.usedAlloc = false;
		int c = _t.tag() ? 1 : 0;
		for (auto const& i: _t)
			if (c++)
			{
				if (us == "LLL" && c == 1)
					code.emplace_back(i, ns, m_readFile);
				else
					code.emplace_back(i, _s, m_readFile);
			}
		auto requireSize = [&](unsigned s) { if (code.size() != s) error<IncorrectParameterCount>(us); };
		auto requireMinSize = [&](unsigned s) { if (code.size() < s) error<IncorrectParameterCount>(us); };
		auto requireMaxSize = [&](unsigned s) { if (code.size() > s) error<IncorrectParameterCount>(us); };
		auto requireDeposit = [&](unsigned i, int s) { if (code[i].m_asm.deposit() != s) error<InvalidDeposit>(us); };

		if (_s.macros.count(make_pair(s, code.size())))
		{
			Macro const& m = _s.macros.at(make_pair(s, code.size()));
			CompilerState cs = _s;
			for (auto const& i: m.env)
				cs.outers[i.first] = i.second;
			for (auto const& i: cs.defs)
				cs.outers[i.first] = i.second;
			cs.defs.clear();
			for (unsigned i = 0; i < m.args.size(); ++i)
			{
				//requireDeposit(i, 1);
				cs.args[m.args[i]] = code[i];
			}
			m_asm.append(CodeFragment(m.code, cs, m_readFile).m_asm);
			for (auto const& i: cs.defs)
				_s.defs[i.first] = i.second;
			for (auto const& i: cs.macros)
				_s.macros.insert(i);
		}
		else if (c_instructions.count(us) && validFunctionalInstruction(us))
		{
			auto it = c_instructions.find(us);
			requireSize(instructionInfo(it->second).args);

			for (unsigned i = code.size(); i; --i)
				m_asm.append(code[i - 1].m_asm, 1);
			m_asm.append(it->second);
		}
		else if (c_arith.count(us))
		{
			auto it = c_arith.find(us);
			requireMinSize(1);
			for (unsigned i = code.size(); i; --i)
			{
				requireDeposit(i - 1, 1);
				m_asm.append(code[i - 1].m_asm, 1);
			}
			for (unsigned i = 1; i < code.size(); ++i)
				m_asm.append(it->second);
		}
		else if (c_binary.count(us))
		{
			auto it = c_binary.find(us);
			requireSize(2);
			requireDeposit(0, 1);
			requireDeposit(1, 1);
			m_asm.append(code[1].m_asm, 1);
			m_asm.append(code[0].m_asm, 1);
			m_asm.append(it->second.first);
			if (it->second.second)
				m_asm.append(Instruction::ISZERO);
		}
		else if (c_unary.count(us))
		{
			auto it = c_unary.find(us);
			requireSize(1);
			requireDeposit(0, 1);
			m_asm.append(code[0].m_asm, 1);
			m_asm.append(it->second);
		}
		else if (us == "IF")
		{
			requireSize(3);
			requireDeposit(0, 1);
			int minDep = min(code[1].m_asm.deposit(), code[2].m_asm.deposit());

			m_asm.append(code[0].m_asm);
			auto mainBranch = m_asm.appendJumpI();

			/// The else branch.
			int startDeposit = m_asm.deposit();
			m_asm.append(code[2].m_asm, minDep);
			auto end = m_asm.appendJump();
			int deposit = m_asm.deposit();
			m_asm.setDeposit(startDeposit);

			/// The main branch.
			m_asm << mainBranch.tag();
			m_asm.append(code[1].m_asm, minDep);
			m_asm << end.tag();
			if (m_asm.deposit() != deposit)
				error<InvalidDeposit>(us);
		}
		else if (us == "WHEN" || us == "UNLESS")
		{
			requireSize(2);
			requireDeposit(0, 1);

			m_asm.append(code[0].m_asm);
			if (us == "WHEN")
				m_asm.append(Instruction::ISZERO);
			auto end = m_asm.appendJumpI();
			m_asm.append(code[1].m_asm, 0);
			m_asm << end.tag();
		}
		else if (us == "WHILE" || us == "UNTIL")
		{
			requireSize(2);
			requireDeposit(0, 1);

			auto begin = m_asm.append(m_asm.newTag());
			m_asm.append(code[0].m_asm);
			if (us == "WHILE")
				m_asm.append(Instruction::ISZERO);
			auto end = m_asm.appendJumpI();
			m_asm.append(code[1].m_asm, 0);
			m_asm.appendJump(begin);
			m_asm << end.tag();
		}
		else if (us == "FOR")
		{
			requireSize(4);
			requireDeposit(1, 1);

			m_asm.append(code[0].m_asm, 0);
			auto begin = m_asm.append(m_asm.newTag());
			m_asm.append(code[1].m_asm);
			m_asm.append(Instruction::ISZERO);
			auto end = m_asm.appendJumpI();
			m_asm.append(code[3].m_asm, 0);
			m_asm.append(code[2].m_asm, 0);
			m_asm.appendJump(begin);
			m_asm << end.tag();
		}
		else if (us == "SWITCH")
		{
			requireMinSize(1);

			bool hasDefault = (code.size() % 2 == 1);
			int startDeposit = m_asm.deposit();
			int targetDeposit = hasDefault ? code[code.size() - 1].m_asm.deposit() : 0;

			// The conditions
			eth::AssemblyItems jumpTags;
			for (unsigned i = 0; i < code.size() - 1; i += 2)
			{
				requireDeposit(i, 1);
				m_asm.append(code[i].m_asm);
				jumpTags.push_back(m_asm.appendJumpI());
			}

			// The default, if present
			if (hasDefault)
				m_asm.append(code[code.size() - 1].m_asm);

			// The targets - appending in reverse makes the top case the most efficient.
			if (code.size() > 1)
			{
				auto end = m_asm.appendJump();
				for (int i = 2 * (code.size() / 2 - 1); i >= 0; i -= 2)
				{
					m_asm << jumpTags[i / 2].tag();
					requireDeposit(i + 1, targetDeposit);
					m_asm.append(code[i + 1].m_asm);
					if (i != 0)
						m_asm.appendJump(end);
				}
				m_asm << end.tag();
			}

			m_asm.setDeposit(startDeposit + targetDeposit);
		}
		else if (us == "ALLOC")
		{
			requireSize(1);
			requireDeposit(0, 1);

			// (alloc N):
			//  - Evaluates to (msize) before the allocation - the start of the allocated memory
			//  - Does not allocate memory when N is zero
			//  - Size of memory allocated is N bytes rounded up to a multiple of 32
			//  - Uses MLOAD to expand MSIZE to avoid modifying memory.

			auto end = m_asm.newTag();
			m_asm.append(Instruction::MSIZE); // Result will be original top of memory
			m_asm.append(code[0].m_asm, 1);   // The alloc argument N
			m_asm.append(Instruction::DUP1);
			m_asm.append(Instruction::ISZERO);// (alloc 0) does not change MSIZE
			m_asm.appendJumpI(end);
			m_asm.append(u256(1));
			m_asm.append(Instruction::DUP2);  // Copy N
			m_asm.append(Instruction::SUB);   // N-1
			m_asm.append(u256(0x1f));         // Bit mask
			m_asm.append(Instruction::NOT);   // Invert
			m_asm.append(Instruction::AND);   // Align N-1 on 32 byte boundary
			m_asm.append(Instruction::MSIZE); // MSIZE is cheap
			m_asm.append(Instruction::ADD);
			m_asm.append(Instruction::MLOAD); // Updates MSIZE
			m_asm.append(Instruction::POP);   // Discard the result of the MLOAD
			m_asm.append(end);
			m_asm.append(Instruction::POP);   // Discard duplicate N

			_s.usedAlloc = true;
		}
		else if (us == "LLL")
		{
			requireMinSize(2);
			requireMaxSize(3);
			requireDeposit(1, 1);

			auto subPush = m_asm.appendSubroutine(make_shared<eth::Assembly>(code[0].assembly(ns)));
			m_asm.append(Instruction::DUP1);
			if (code.size() == 3)
			{
				requireDeposit(2, 1);
				m_asm.append(code[2].m_asm, 1);
				m_asm.append(Instruction::LT);
				m_asm.append(Instruction::ISZERO);
				m_asm.append(Instruction::MUL);
				m_asm.append(Instruction::DUP1);
			}
			m_asm.append(subPush);
			m_asm.append(code[1].m_asm, 1);
			m_asm.append(Instruction::CODECOPY);
		}
		else if (us == "&&" || us == "||")
		{
			requireMinSize(1);
			for (unsigned i = 0; i < code.size(); ++i)
				requireDeposit(i, 1);

			auto end = m_asm.newTag();
			if (code.size() > 1)
			{
				m_asm.append((u256)(us == "||" ? 1 : 0));
				for (unsigned i = 1; i < code.size(); ++i)
				{
					// Check if true - predicate
					m_asm.append(code[i - 1].m_asm, 1);
					if (us == "&&")
						m_asm.append(Instruction::ISZERO);
					m_asm.appendJumpI(end);
				}
				m_asm.append(Instruction::POP);
			}

			// Check if true - predicate
			m_asm.append(code.back().m_asm, 1);

			// At end now.
			m_asm.append(end);
		}
		else if (us == "SEQ")
		{
			unsigned ii = 0;
			for (auto const& i: code)
				if (++ii < code.size())
					m_asm.append(i.m_asm, 0);
				else
					m_asm.append(i.m_asm);
		}
		else if (us == "RAW")
		{
			for (auto const& i: code)
				m_asm.append(i.m_asm);
			// Leave only the last item on stack.
			while (m_asm.deposit() > 1)
				m_asm.append(Instruction::POP);
		}
		else if (us == "BYTECODESIZE")
		{
			m_asm.appendProgramSize();
		}
		else if (us.find_first_of("1234567890") != 0 && us.find_first_not_of("QWERTYUIOPASDFGHJKLZXCVBNM1234567890_-") == string::npos)
			m_asm.append((u256)varAddress(s));
		else
			error<InvalidOperation>("Unsupported keyword: '" + us + "'");
	}
}

CodeFragment CodeFragment::compile(string const& _src, CompilerState& _s, ReadCallback const& _readFile)
{
	CodeFragment ret;
	sp::utree o;
	parseTreeLLL(_src, o);
	if (!o.empty())
		ret = CodeFragment(o, _s, _readFile);
	_s.treesToKill.push_back(o);
	return ret;
}
