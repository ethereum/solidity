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
/** @file Assembly.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "Assembly.h"

#include <libdevcore/Log.h>

using namespace std;
using namespace dev;
using namespace dev::eth;

unsigned AssemblyItem::bytesRequired(unsigned _addressLength) const
{
	switch (m_type)
	{
	case Operation:
	case Tag: // 1 byte for the JUMPDEST
		return 1;
	case PushString:
		return 33;
	case Push:
		return 1 + max<unsigned>(1, dev::bytesRequired(m_data));
	case PushSubSize:
	case PushProgramSize:
		return 4;		// worst case: a 16MB program
	case PushTag:
	case PushData:
	case PushSub:
		return 1 + _addressLength;
	case NoOptimizeBegin:
	case NoOptimizeEnd:
		return 0;
	default:
		break;
	}
	BOOST_THROW_EXCEPTION(InvalidOpcode());
}

int AssemblyItem::deposit() const
{
	switch (m_type)
	{
	case Operation:
		return instructionInfo((Instruction)(byte)m_data).ret - instructionInfo((Instruction)(byte)m_data).args;
	case Push:
	case PushString:
	case PushTag:
	case PushData:
	case PushSub:
	case PushSubSize:
	case PushProgramSize:
		return 1;
	case Tag:
		return 0;
	default:;
	}
	return 0;
}

unsigned Assembly::bytesRequired() const
{
	for (unsigned br = 1;; ++br)
	{
		unsigned ret = 1;
		for (auto const& i: m_data)
			ret += i.second.size();

		for (AssemblyItem const& i: m_items)
			ret += i.bytesRequired(br);
		if (dev::bytesRequired(ret) <= br)
			return ret;
	}
}

void Assembly::append(Assembly const& _a)
{
	auto newDeposit = m_deposit + _a.deposit();
	for (AssemblyItem i: _a.m_items)
	{
		if (i.type() == Tag || i.type() == PushTag)
			i.m_data += m_usedTags;
		append(i);
	}
	m_deposit = newDeposit;
	m_usedTags += _a.m_usedTags;
	for (auto const& i: _a.m_data)
		m_data.insert(i);
	for (auto const& i: _a.m_strings)
		m_strings.insert(i);
	for (auto const& i: _a.m_subs)
		m_subs.insert(i);

	assert(!_a.m_baseDeposit);
	assert(!_a.m_totalDeposit);
}

void Assembly::append(Assembly const& _a, int _deposit)
{
	if (_deposit > _a.m_deposit)
		BOOST_THROW_EXCEPTION(InvalidDeposit());
	else
	{
		append(_a);
		while (_deposit++ < _a.m_deposit)
			append(Instruction::POP);
	}
}

ostream& dev::eth::operator<<(ostream& _out, AssemblyItemsConstRef _i)
{
	for (AssemblyItem const& i: _i)
		switch (i.type())
		{
		case Operation:
			_out << " " << instructionInfo((Instruction)(byte)i.data()).name;
			break;
		case Push:
			_out << " PUSH" << i.data();
			break;
		case PushString:
			_out << " PUSH'[" << hex << (unsigned)i.data() << "]";
			break;
		case PushTag:
			_out << " PUSH[tag" << i.data() << "]";
			break;
		case Tag:
			_out << " tag" << i.data() << ": JUMPDEST";
			break;
		case PushData:
			_out << " PUSH*[" << hex << (unsigned)i.data() << "]";
			break;
		case PushSub:
			_out << " PUSHs[" << hex << h256(i.data()).abridged() << "]";
			break;
		case PushSubSize:
			_out << " PUSHss[" << hex << h256(i.data()).abridged() << "]";
			break;
		case PushProgramSize:
			_out << " PUSHSIZE";
			break;
		case NoOptimizeBegin:
			_out << " DoNotOptimze{{";
			break;
		case NoOptimizeEnd:
			_out << " DoNotOptimze}}";
			break;
		case UndefinedItem:
			_out << " ???";
			break;
		default:
			BOOST_THROW_EXCEPTION(InvalidOpcode());
		}
	return _out;
}

ostream& Assembly::streamRLP(ostream& _out, string const& _prefix) const
{
	_out << _prefix << ".code:" << endl;
	for (AssemblyItem const& i: m_items)
		switch (i.m_type)
		{
		case Operation:
			_out << _prefix << "  " << instructionInfo((Instruction)(byte)i.m_data).name << endl;
			break;
		case Push:
			_out << _prefix << "  PUSH " << i.m_data << endl;
			break;
		case PushString:
			_out << _prefix << "  PUSH \"" << m_strings.at((h256)i.m_data) << "\"" << endl;
			break;
		case PushTag:
			_out << _prefix << "  PUSH [tag" << i.m_data << "]" << endl;
			break;
		case PushSub:
			_out << _prefix << "  PUSH [$" << h256(i.m_data).abridged() << "]" << endl;
			break;
		case PushSubSize:
			_out << _prefix << "  PUSH #[$" << h256(i.m_data).abridged() << "]" << endl;
			break;
		case PushProgramSize:
			_out << _prefix << "  PUSHSIZE" << endl;
			break;
		case Tag:
			_out << _prefix << "tag" << i.m_data << ": " << endl << _prefix << "  JUMPDEST" << endl;
			break;
		case PushData:
			_out << _prefix << "  PUSH [" << hex << (unsigned)i.m_data << "]" << endl;
			break;
		case NoOptimizeBegin:
			_out << _prefix << "DoNotOptimze{{" << endl;
			break;
		case NoOptimizeEnd:
			_out << _prefix << "DoNotOptimze}}" << endl;
			break;
		default:
			BOOST_THROW_EXCEPTION(InvalidOpcode());
		}

	if (m_data.size() || m_subs.size())
	{
		_out << _prefix << ".data:" << endl;
		for (auto const& i: m_data)
			if (!m_subs.count(i.first))
				_out << _prefix << "  " << hex << (unsigned)(u256)i.first << ": " << toHex(i.second) << endl;
		for (auto const& i: m_subs)
		{
			_out << _prefix << "  " << hex << (unsigned)(u256)i.first << ": " << endl;
			i.second.streamRLP(_out, _prefix + "  ");
		}
	}
	return _out;
}

AssemblyItem const& Assembly::append(AssemblyItem const& _i)
{
	m_deposit += _i.deposit();
	m_items.push_back(_i);
	return back();
}

void Assembly::injectStart(AssemblyItem const& _i)
{
	m_items.insert(m_items.begin(), _i);
}

inline bool matches(AssemblyItemsConstRef _a, AssemblyItemsConstRef _b)
{
	if (_a.size() != _b.size())
		return false;
	for (unsigned i = 0; i < _a.size(); ++i)
		if (!_a[i].match(_b[i]))
			return false;
	return true;
}

inline bool popCountIncreased(AssemblyItemsConstRef _pre, AssemblyItems const& _post)
{
	auto isPop = [](AssemblyItem const& _item) -> bool { return _item.match(AssemblyItem(Instruction::POP)); };
	return count_if(begin(_post), end(_post), isPop) > count_if(begin(_pre), end(_pre), isPop);
}

//@todo this has to move to a special optimizer class soon
template<class Iterator>
unsigned bytesRequiredBySlice(Iterator _begin, Iterator _end)
{
	// this is only used in the optimizer, so we can provide a guess for the address length
	unsigned addressLength = 4;
	unsigned size = 0;
	for (; _begin != _end; ++_begin)
		size += _begin->bytesRequired(addressLength);
	return size;
}

struct OptimiserChannel: public LogChannel { static const char* name() { return "OPT"; } static const int verbosity = 12; };
#define copt dev::LogOutputStream<OptimiserChannel, true>()

Assembly& Assembly::optimise(bool _enable)
{
	if (!_enable)
		return *this;
	auto signextend = [](u256 a, u256 b) -> u256
	{
		if (a >= 31)
			return b;
		unsigned testBit = unsigned(a) * 8 + 7;
		u256 mask = (u256(1) << testBit) - 1;
		return boost::multiprecision::bit_test(b, testBit) ? b | ~mask : b & mask;
	};
	map<Instruction, function<u256(u256, u256)>> const c_simple =
	{
		{ Instruction::SUB, [](u256 a, u256 b)->u256{return a - b;} },
		{ Instruction::DIV, [](u256 a, u256 b)->u256{return a / b;} },
		{ Instruction::SDIV, [](u256 a, u256 b)->u256{return s2u(u2s(a) / u2s(b));} },
		{ Instruction::MOD, [](u256 a, u256 b)->u256{return a % b;} },
		{ Instruction::SMOD, [](u256 a, u256 b)->u256{return s2u(u2s(a) % u2s(b));} },
		{ Instruction::EXP, [](u256 a, u256 b)->u256{return (u256)boost::multiprecision::powm((bigint)a, (bigint)b, bigint(1) << 256);} },
		{ Instruction::SIGNEXTEND, signextend },
		{ Instruction::LT, [](u256 a, u256 b)->u256{return a < b ? 1 : 0;} },
		{ Instruction::GT, [](u256 a, u256 b)->u256{return a > b ? 1 : 0;} },
		{ Instruction::SLT, [](u256 a, u256 b)->u256{return u2s(a) < u2s(b) ? 1 : 0;} },
		{ Instruction::SGT, [](u256 a, u256 b)->u256{return u2s(a) > u2s(b) ? 1 : 0;} },
		{ Instruction::EQ, [](u256 a, u256 b)->u256{return a == b ? 1 : 0;} },
	};
	map<Instruction, function<u256(u256, u256)>> const c_associative =
	{
		{ Instruction::ADD, [](u256 a, u256 b)->u256{return a + b;} },
		{ Instruction::MUL, [](u256 a, u256 b)->u256{return a * b;} },
		{ Instruction::AND, [](u256 a, u256 b)->u256{return a & b;} },
		{ Instruction::OR, [](u256 a, u256 b)->u256{return a | b;} },
		{ Instruction::XOR, [](u256 a, u256 b)->u256{return a ^ b;} },
	};
	std::vector<pair<AssemblyItem, u256>> const c_identities =
	{ { Instruction::ADD, 0}, { Instruction::MUL, 1}, { Instruction::MOD, 0}, { Instruction::OR, 0}, { Instruction::XOR, 0} };
	std::vector<pair<AssemblyItems, function<AssemblyItems(AssemblyItemsConstRef)>>> rules =
	{
		{ { Push, Instruction::POP }, [](AssemblyItemsConstRef) -> AssemblyItems { return {}; } },
		{ { PushTag, Instruction::POP }, [](AssemblyItemsConstRef) -> AssemblyItems { return {}; } },
		{ { PushString, Instruction::POP }, [](AssemblyItemsConstRef) -> AssemblyItems { return {}; } },
		{ { PushSub, Instruction::POP }, [](AssemblyItemsConstRef) -> AssemblyItems { return {}; } },
		{ { PushSubSize, Instruction::POP }, [](AssemblyItemsConstRef) -> AssemblyItems { return {}; } },
		{ { PushProgramSize, Instruction::POP }, [](AssemblyItemsConstRef) -> AssemblyItems { return {}; } },
		{ { Push, PushTag, Instruction::JUMPI }, [](AssemblyItemsConstRef m) -> AssemblyItems { if (m[0].data()) return { m[1], Instruction::JUMP }; else return {}; } },
		{ { Instruction::ISZERO, Instruction::ISZERO }, [](AssemblyItemsConstRef) -> AssemblyItems { return {}; } },
	};

	for (auto const& i: c_simple)
		rules.push_back({ { Push, Push, i.first }, [&](AssemblyItemsConstRef m) -> AssemblyItems { return { i.second(m[1].data(), m[0].data()) }; } });
	for (auto const& i: c_associative)
	{
		rules.push_back({ { Push, Push, i.first }, [&](AssemblyItemsConstRef m) -> AssemblyItems { return { i.second(m[1].data(), m[0].data()) }; } });
		rules.push_back({ { Push, i.first, Push, i.first }, [&](AssemblyItemsConstRef m) -> AssemblyItems { return { i.second(m[2].data(), m[0].data()), i.first }; } });
	}
	for (auto const& i: c_identities)
		rules.push_back({{Push, i.first}, [&](AssemblyItemsConstRef m) -> AssemblyItems
							{ return m[0].data() == i.second ? AssemblyItems() : m.toVector(); }});
	// jump to next instruction
	rules.push_back({ { PushTag, Instruction::JUMP, Tag }, [](AssemblyItemsConstRef m) -> AssemblyItems { if (m[0].m_data == m[2].m_data) return {m[2]}; else return m.toVector(); }});

	// pop optimization, do not compute values that are popped again anyway
	rules.push_back({ { AssemblyItem(UndefinedItem), Instruction::POP }, [](AssemblyItemsConstRef m) -> AssemblyItems
					  {
						  if (m[0].type() != Operation)
							return m.toVector();
						  Instruction instr = Instruction(byte(m[0].data()));
						  if (Instruction::DUP1 <= instr && instr <= Instruction::DUP16)
							return {};
						  InstructionInfo info = instructionInfo(instr);
						  if (info.sideEffects || info.additional != 0 || info.ret != 1)
							  return m.toVector();
						  return AssemblyItems(info.args, Instruction::POP);
					  } });
	// compute constants close to powers of two by expressions
	auto computeConstants = [](AssemblyItemsConstRef m) -> AssemblyItems
	{
		u256 const& c = m[0].data();
		unsigned const minBits = 4 * 8;
		if (c < (bigint(1) << minBits))
			return m.toVector(); // we need at least "PUSH1 <bits> PUSH1 <2> EXP"
		if (c == u256(-1))
			return {u256(0), Instruction::NOT};
		for (unsigned bits = minBits; bits < 256; ++bits)
		{
			bigint const diff = c - (bigint(1) << bits);
			if (abs(diff) > 0xff)
				continue;
			AssemblyItems powerOfTwo{u256(bits), u256(2), Instruction::EXP};
			if (diff == 0)
				return powerOfTwo;
			return AssemblyItems{u256(abs(diff))} + powerOfTwo +
				   AssemblyItems{diff > 0 ? Instruction::ADD : Instruction::SUB};
		}
		return m.toVector();
	};
	rules.push_back({{Push}, computeConstants});

	copt << *this;

	unsigned total = 0;
	for (unsigned count = 1; count > 0; total += count)
	{
		count = 0;
		for (unsigned i = 0; i < m_items.size(); ++i)
		{
			if (m_items[i].type() == NoOptimizeBegin)
			{
				while (i < m_items.size() && m_items[i].type() != NoOptimizeEnd)
					++i;
				continue;
			}
			for (auto const& r: rules)
			{
				auto vr = AssemblyItemsConstRef(&m_items).cropped(i, r.first.size());
				if (matches(vr, &r.first))
				{
					auto rw = r.second(vr);
					unsigned const vrSizeInBytes = bytesRequiredBySlice(vr.begin(), vr.end());
					unsigned const rwSizeInBytes = bytesRequiredBySlice(rw.begin(), rw.end());
					if (rwSizeInBytes < vrSizeInBytes || (rwSizeInBytes == vrSizeInBytes && popCountIncreased(vr, rw)))
					{
						copt << vr << "matches" << AssemblyItemsConstRef(&r.first) << "becomes...";
						copt << AssemblyItemsConstRef(&rw);
						if (rw.size() > vr.size())
						{
							// create hole in the vector
							unsigned sizeIncrease = rw.size() - vr.size();
							m_items.resize(m_items.size() + sizeIncrease, AssemblyItem(UndefinedItem));
							move_backward(m_items.begin() + i, m_items.end() - sizeIncrease, m_items.end());
						}
						else
							m_items.erase(m_items.begin() + i + rw.size(), m_items.begin() + i + vr.size());

						copy(rw.begin(), rw.end(), m_items.begin() + i);

						count++;
						copt << "Now:\n" << m_items;
					}
				}
			}
			if (m_items[i].type() == Operation && m_items[i].data() == (byte)Instruction::JUMP)
			{
				bool o = false;
				while (m_items.size() > i + 1 && m_items[i + 1].type() != Tag)
				{
					if (m_items[i + 1].type() == NoOptimizeBegin)
						break;
					m_items.erase(m_items.begin() + i + 1);
					o = true;
				}
				if (o)
				{
					copt << "Jump with no tag. Now:\n" << m_items;
					++count;
				}
			}
		}

		map<u256, unsigned> tags;
		for (unsigned i = 0; i < m_items.size(); ++i)
			if (m_items[i].type() == Tag)
				tags.insert(make_pair(m_items[i].data(), i));

		for (auto const& i: m_items)
			if (i.type() == PushTag)
				tags.erase(i.data());

		if (tags.size())
		{
			auto t = *tags.begin();
			unsigned i = t.second;
			if (i && m_items[i - 1].type() == Operation && m_items[i - 1].data() == (byte)Instruction::JUMP)
				while (i < m_items.size() && (m_items[i].type() != Tag || tags.count(m_items[i].data())))
				{
					if (m_items[i].type() == Tag && tags.count(m_items[i].data()))
						tags.erase(m_items[i].data());
					m_items.erase(m_items.begin() + i);
				}
			else
			{
				m_items.erase(m_items.begin() + i);
				tags.erase(t.first);
			}
			copt << "Unused tag. Now:\n" << m_items;
			++count;
		}
	}

	copt << total << " optimisations done.";

	for (auto& i: m_subs)
	  i.second.optimise(true);

	return *this;
}

bytes Assembly::assemble() const
{
	bytes ret;

	unsigned totalBytes = bytesRequired();
	vector<unsigned> tagPos(m_usedTags);
	map<unsigned, unsigned> tagRef;
	multimap<h256, unsigned> dataRef;
	vector<unsigned> sizeRef; ///< Pointers to code locations where the size of the program is inserted
	unsigned bytesPerTag = dev::bytesRequired(totalBytes);
	byte tagPush = (byte)Instruction::PUSH1 - 1 + bytesPerTag;

	for (auto const& i: m_subs)
		m_data[i.first] = i.second.assemble();

	unsigned bytesRequiredIncludingData = bytesRequired();
	unsigned bytesPerDataRef = dev::bytesRequired(bytesRequiredIncludingData);
	byte dataRefPush = (byte)Instruction::PUSH1 - 1 + bytesPerDataRef;
	ret.reserve(bytesRequiredIncludingData);
	// m_data must not change from here on

	for (AssemblyItem const& i: m_items)
		switch (i.m_type)
		{
		case Operation:
			ret.push_back((byte)i.m_data);
			break;
		case PushString:
		{
			ret.push_back((byte)Instruction::PUSH32);
			unsigned ii = 0;
			for (auto j: m_strings.at((h256)i.m_data))
				if (++ii > 32)
					break;
				else
					ret.push_back((byte)j);
			while (ii++ < 32)
				ret.push_back(0);
			break;
		}
		case Push:
		{
			byte b = max<unsigned>(1, dev::bytesRequired(i.m_data));
			ret.push_back((byte)Instruction::PUSH1 - 1 + b);
			ret.resize(ret.size() + b);
			bytesRef byr(&ret.back() + 1 - b, b);
			toBigEndian(i.m_data, byr);
			break;
		}
		case PushTag:
		{
			ret.push_back(tagPush);
			tagRef[ret.size()] = (unsigned)i.m_data;
			ret.resize(ret.size() + bytesPerTag);
			break;
		}
		case PushData: case PushSub:
		{
			ret.push_back(dataRefPush);
			dataRef.insert(make_pair((h256)i.m_data, ret.size()));
			ret.resize(ret.size() + bytesPerDataRef);
			break;
		}
		case PushSubSize:
		{
			auto s = m_data[i.m_data].size();
			byte b = max<unsigned>(1, dev::bytesRequired(s));
			ret.push_back((byte)Instruction::PUSH1 - 1 + b);
			ret.resize(ret.size() + b);
			bytesRef byr(&ret.back() + 1 - b, b);
			toBigEndian(s, byr);
			break;
		}
		case PushProgramSize:
		{
			ret.push_back(dataRefPush);
			sizeRef.push_back(ret.size());
			ret.resize(ret.size() + bytesPerDataRef);
			break;
		}
		case Tag:
			tagPos[(unsigned)i.m_data] = ret.size();
			ret.push_back((byte)Instruction::JUMPDEST);
			break;
		case NoOptimizeBegin:
		case NoOptimizeEnd:
			break;
		default:
			BOOST_THROW_EXCEPTION(InvalidOpcode());
		}

	for (auto const& i: tagRef)
	{
		bytesRef r(ret.data() + i.first, bytesPerTag);
		toBigEndian(tagPos[i.second], r);
	}

	if (m_data.size())
	{
		ret.push_back(0);
		for (auto const& i: m_data)
		{
			auto its = dataRef.equal_range(i.first);
			if (its.first != its.second)
			{
				for (auto it = its.first; it != its.second; ++it)
				{
					bytesRef r(ret.data() + it->second, bytesPerDataRef);
					toBigEndian(ret.size(), r);
				}
				for (auto b: i.second)
					ret.push_back(b);
			}
		}
	}
	for (unsigned pos: sizeRef)
	{
		bytesRef r(ret.data() + pos, bytesPerDataRef);
		toBigEndian(ret.size(), r);
	}
	return ret;
}
