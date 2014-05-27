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

#include <libethcore/CommonEth.h>

using namespace std;
using namespace eth;

int AssemblyItem::deposit() const
{
	switch (m_type)
	{
	case Operation:
		return c_instructionInfo.at((Instruction)(byte)m_data).ret - c_instructionInfo.at((Instruction)(byte)m_data).args;
	case Push: case PushString: case PushTag: case PushData:
		return 1;
	case Tag:
		return 0;
	}
	assert(false);
}

unsigned Assembly::bytesRequired() const
{
	for (unsigned br = 1;; ++br)
	{
		unsigned ret = 1;
		for (auto const& i: m_data)
			ret += i.second.size();

		for (AssemblyItem const& i: m_items)
			switch (i.m_type)
			{
			case Operation:
				ret++;
				break;
			case PushString:
				ret += 33;
				break;
			case Push:
				ret += 1 + max<unsigned>(1, eth::bytesRequired(i.m_data));
				break;
			case PushTag:
			case PushData:
				ret += 1 + br;
			case Tag:;
			}
		if (eth::bytesRequired(ret) <= br)
			return ret;
	}
}

void Assembly::append(Assembly const& _a)
{
	for (AssemblyItem i: _a.m_items)
	{
		if (i.type() == Tag || i.type() == PushTag)
			i.m_data += m_usedTags;
		append(i);
	}
	m_usedTags += _a.m_usedTags;
	for (auto const& i: _a.m_data)
		m_data.insert(i);
	for (auto const& i: _a.m_strings)
		m_strings.insert(i);

	assert(!_a.m_baseDeposit);
	assert(!_a.m_totalDeposit);
}

void Assembly::append(Assembly const& _a, int _deposit)
{
	if (_deposit > _a.m_deposit)
		throw InvalidDeposit();
	else
	{
		append(_a);
		while (_deposit++ < _a.m_deposit)
			append(Instruction::POP);
	}
}

ostream& Assembly::streamOut(ostream& _out) const
{
	_out << ".code:" << endl;
	for (AssemblyItem const& i: m_items)
		switch (i.m_type)
		{
		case Operation:
			_out << "  " << c_instructionInfo.at((Instruction)(byte)i.m_data).name << endl;
			break;
		case Push:
			_out << "  PUSH " << i.m_data << endl;
			break;
		case PushString:
			_out << "  PUSH \"" << m_strings.at((h256)i.m_data) << "\"" << endl;
			break;
		case PushTag:
			_out << "  PUSH [tag" << i.m_data << "]" << endl;
			break;
		case Tag:
			_out << "tag" << i.m_data << ": " << endl;
			break;
		case PushData:
			_out << "  PUSH [" << h256(i.m_data).abridged() << "]" << endl;
			break;
		}

	if (m_data.size())
	{
		_out << ".data:" << endl;
		for (auto const& i: m_data)
			_out << "  " << i.first.abridged() << ": " << toHex(i.second) << endl;
	}
	return _out;
}

AssemblyItem const& Assembly::append(AssemblyItem const& _i)
{
	m_deposit += _i.deposit();
	m_items.push_back(_i);
	return back();
}

void Assembly::optimise()
{
	std::vector<pair<  vector<int>,  function< vector<AssemblyItem>(vector<AssemblyItem>) >  >> rules;
//	rules.insert(make_pair({(int)Instruction::ADD, (int)Instruction::ADD, -(int)Push}, []() {}));
}

bytes Assembly::assemble() const
{
	bytes ret;

	unsigned totalBytes = bytesRequired();
	ret.reserve(totalBytes);
	vector<unsigned> tagPos(m_usedTags);
	map<unsigned, unsigned> tagRef;
	multimap<h256, unsigned> dataRef;
	unsigned bytesPerTag = eth::bytesRequired(totalBytes);
	byte tagPush = (byte)Instruction::PUSH1 - 1 + bytesPerTag;

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
			byte b = max<unsigned>(1, eth::bytesRequired(i.m_data));
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
		case PushData:
		{
			ret.push_back(tagPush);
			dataRef.insert(make_pair((h256)i.m_data, ret.size()));
			ret.resize(ret.size() + bytesPerTag);
			break;
		}
		case Tag:
			tagPos[(unsigned)i.m_data] = ret.size();
			break;
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
			for (auto it = its.first; it != its.second; ++it)
			{
				bytesRef r(ret.data() + it->second, bytesPerTag);
				toBigEndian(ret.size(), r);
			}
			for (auto b: i.second)
				ret.push_back(b);
		}
	}
	return ret;
}
