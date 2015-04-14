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
#include <fstream>
#include <libdevcore/Log.h>
#include <libevmcore/CommonSubexpressionEliminator.h>
#include <libevmcore/ControlFlowGraph.h>
#include <json/json.h>

using namespace std;
using namespace dev;
using namespace dev::eth;

void Assembly::append(Assembly const& _a)
{
	auto newDeposit = m_deposit + _a.deposit();
	for (AssemblyItem i: _a.m_items)
	{
		if (i.type() == Tag || i.type() == PushTag)
			i.setData(i.data() + m_usedTags);
		else if (i.type() == PushSub || i.type() == PushSubSize)
			i.setData(i.data() + m_usedTags);
		append(i);
	}
	m_deposit = newDeposit;
	m_usedTags += _a.m_usedTags;
	for (auto const& i: _a.m_data)
		m_data.insert(i);
	for (auto const& i: _a.m_strings)
		m_strings.insert(i);
	for (auto const& i: _a.m_subs)
		m_subs.push_back(i);

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

string Assembly::getLocationFromSources(StringMap const& _sourceCodes, SourceLocation const& _location) const
{
	if (_location.isEmpty() || _sourceCodes.empty() || _location.start >= _location.end || _location.start < 0)
		return "";

	auto it = _sourceCodes.find(*_location.sourceName);
	if (it == _sourceCodes.end())
		return "";

	string const& source = it->second;
	if (size_t(_location.start) >= source.size())
		return "";

	string cut = source.substr(_location.start, _location.end - _location.start);
	auto newLinePos = cut.find_first_of("\n");
	if (newLinePos != string::npos)
		cut = cut.substr(0, newLinePos) + "...";

	return move(cut);
}

ostream& Assembly::streamAsm(ostream& _out, string const& _prefix, StringMap const& _sourceCodes) const
{
	_out << _prefix << ".code:" << endl;
	for (AssemblyItem const& i: m_items)
	{
		_out << _prefix;
		switch (i.type())
		{
		case Operation:
			_out << "  " << instructionInfo(i.instruction()).name  << "\t" << i.getJumpTypeAsString();
			break;
		case Push:
			_out << "  PUSH " << i.data();
			break;
		case PushString:
			_out << "  PUSH \"" << m_strings.at((h256)i.data()) << "\"";
			break;
		case PushTag:
			_out << "  PUSH [tag" << i.data() << "]";
			break;
		case PushSub:
			_out << "  PUSH [$" << h256(i.data()).abridged() << "]";
			break;
		case PushSubSize:
			_out << "  PUSH #[$" << h256(i.data()).abridged() << "]";
			break;
		case PushProgramSize:
			_out << "  PUSHSIZE";
			break;
		case Tag:
			_out << "tag" << i.data() << ": " << endl << _prefix << "  JUMPDEST";
			break;
		case PushData:
			_out << "  PUSH [" << hex << (unsigned)i.data() << "]";
			break;
		default:
			BOOST_THROW_EXCEPTION(InvalidOpcode());
		}
		_out << "\t\t" << getLocationFromSources(_sourceCodes, i.getLocation()) << endl;
	}

	if (!m_data.empty() || !m_subs.empty())
	{
		_out << _prefix << ".data:" << endl;
		for (auto const& i: m_data)
			if (u256(i.first) >= m_subs.size())
				_out << _prefix << "  " << hex << (unsigned)(u256)i.first << ": " << toHex(i.second) << endl;
		for (size_t i = 0; i < m_subs.size(); ++i)
		{
			_out << _prefix << "  " << hex << i << ": " << endl;
			m_subs[i].stream(_out, _prefix + "  ", _sourceCodes);
		}
	}
	return _out;
}

Json::Value Assembly::createJsonValue(string _name, int _locationX, int _locationY, string _value, string _jumpType) const
{
	Json::Value value;
	assert(!_name.empty());
	value["name"] = _name;
	value["locationX"] = _locationX;
	value["locationY"] = _locationY;
	if (!_value.empty())
		value["value"] = _value;
	if (!_jumpType.empty())
		value["jumpType"] = _jumpType;
	return move(value);
}

ostream& Assembly::streamAsmJson(ostream& _out, string const& _prefix, StringMap const& _sourceCodes, bool _inJsonFormat) const
{
	Json::Value root;

	string currentArrayName = ".code";
	std::vector<Json::Value> currentCollection;
	for (AssemblyItem const& i: m_items)
	{
		switch (i.type())
		{
		case Operation:
			currentCollection.push_back(createJsonValue(instructionInfo(i.instruction()).name, i.getLocation().start, i.getLocation().end, i.getJumpTypeAsString()));
			break;
		case Push:
			currentCollection.push_back(createJsonValue(string("PUSH"), i.getLocation().start, i.getLocation().end, string(i.data()), i.getJumpTypeAsString()));
			break;
		case PushString:
			currentCollection.push_back(createJsonValue(string("PUSH tag"), i.getLocation().start, i.getLocation().end, m_strings.at((h256)i.data())));
			break;
		case PushTag:
			currentCollection.push_back(createJsonValue(string("PUSH [tag]"), i.getLocation().start, i.getLocation().end, string(i.data())));
			break;
		case PushSub:
			currentCollection.push_back(createJsonValue(string("PUSH"), i.getLocation().start, i.getLocation().end, string("[$]" + string(h256(i.data()).abridged()))));
			break;
		case PushSubSize:
			currentCollection.push_back(createJsonValue(string("PUSH"), i.getLocation().start, i.getLocation().end, string("#[$]" + string(h256(i.data()).abridged()))));
			break;
		case PushProgramSize:
			currentCollection.push_back(createJsonValue(string("PUSHSIZE"), i.getLocation().start, i.getLocation().end));
			break;
		case Tag:
			{
				Json::Value collection(Json::arrayValue);
				for (auto it: currentCollection)
					collection.append(it);
				currentCollection.clear();
				root[currentArrayName] = collection;
				currentArrayName = "tag" + string(i.data());
				Json::Value jumpdest;
				jumpdest["name"] = "JUMDEST";
				currentCollection.push_back(jumpdest);
			}
			break;
		case PushData:
			{
				Json::Value pushData;
				pushData["name"] = "PUSH hex";
				std::stringstream hexStr;
				hexStr << hex << (unsigned)i.data();
				currentCollection.push_back(createJsonValue(string("PUSH hex"), i.getLocation().start, i.getLocation().end, hexStr.str()));
			}
			break;
		default:
			BOOST_THROW_EXCEPTION(InvalidOpcode());
		}
	}

	//todo check if the last was tag
	Json::Value collection(Json::arrayValue);
	for (auto it: currentCollection)
		collection.append(it);
	root[currentArrayName] = collection;

	Json::Value rootData;
	if (!m_data.empty() || !m_subs.empty())
	{
		Json::Value dataCollection(Json::arrayValue);
		for (auto const& i: m_data)
			if (u256(i.first) >= m_subs.size())
			{
				std::stringstream hexStr;
				hexStr << _prefix << hex << (unsigned)(u256)i.first << ": " << toHex(i.second);
				Json::Value data;
				data["value"] = hexStr.str();
				dataCollection.append(data);
			}
		rootData[_prefix + ".data"] = collection;
		_out << root << rootData;

		for (size_t i = 0; i < m_subs.size(); ++i)
		{
			std::stringstream hexStr;
			hexStr << _prefix << hex << i << ": ";
			//_out << _prefix << "  " << hex << i << ": " << endl;
			//todo check recursion check order.
			m_subs[i].stream(_out, _prefix + "  ", _sourceCodes, _inJsonFormat);
		}
	} else
		_out << root;

	return _out;
}

ostream& Assembly::stream(ostream& _out, string const& _prefix, StringMap const& _sourceCodes, bool _inJsonFormat) const
{
	if (_inJsonFormat)
		return streamAsmJson(_out, _prefix, _sourceCodes, _inJsonFormat);
	else
		return streamAsm(_out, _prefix, _sourceCodes);
}

AssemblyItem const& Assembly::append(AssemblyItem const& _i)
{
	m_deposit += _i.deposit();
	m_items.push_back(_i);
	if (m_items.back().getLocation().isEmpty() && !m_currentSourceLocation.isEmpty())
		m_items.back().setLocation(m_currentSourceLocation);
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

struct OptimiserChannel: public LogChannel { static const char* name() { return "OPT"; } static const int verbosity = 12; };
#define copt dev::LogOutputStream<OptimiserChannel, true>()

Assembly& Assembly::optimise(bool _enable)
{
	if (!_enable)
		return *this;
	std::vector<pair<AssemblyItems, function<AssemblyItems(AssemblyItemsConstRef)>>> rules;
	// jump to next instruction
	rules.push_back({ { PushTag, Instruction::JUMP, Tag }, [](AssemblyItemsConstRef m) -> AssemblyItems { if (m[0].data() == m[2].data()) return {m[2]}; else return m.toVector(); }});

	unsigned total = 0;
	for (unsigned count = 1; count > 0; total += count)
	{
		copt << *this;
		count = 0;

		copt << "Performing control flow analysis...";
		{
			ControlFlowGraph cfg(m_items);
			AssemblyItems optItems = cfg.optimisedItems();
			if (optItems.size() < m_items.size())
			{
				copt << "Old size: " << m_items.size() << ", new size: " << optItems.size();
				m_items = move(optItems);
				count++;
			}
		}

		copt << "Performing common subexpression elimination...";
		for (auto iter = m_items.begin(); iter != m_items.end();)
		{
			CommonSubexpressionEliminator eliminator;
			auto orig = iter;
			iter = eliminator.feedItems(iter, m_items.end());
			AssemblyItems optItems;
			bool shouldReplace = false;
			try
			{
				optItems = eliminator.getOptimizedItems();
				shouldReplace = (optItems.size() < size_t(iter - orig));
			}
			catch (StackTooDeepException const&)
			{
				// This might happen if the opcode reconstruction is not as efficient
				// as the hand-crafted code.
			}

			if (shouldReplace)
			{
				copt << "Old size: " << (iter - orig) << ", new size: " << optItems.size();
				count++;
				for (auto moveIter = optItems.begin(); moveIter != optItems.end(); ++orig, ++moveIter)
					*orig = move(*moveIter);
				iter = m_items.erase(orig, iter);
			}
		}
	}

	copt << total << " optimisations done.";

	for (auto& sub: m_subs)
	  sub.optimise(true);

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

	for (size_t i = 0; i < m_subs.size(); ++i)
		m_data[u256(i)] = m_subs[i].assemble();

	unsigned bytesRequiredIncludingData = bytesRequired();
	unsigned bytesPerDataRef = dev::bytesRequired(bytesRequiredIncludingData);
	byte dataRefPush = (byte)Instruction::PUSH1 - 1 + bytesPerDataRef;
	ret.reserve(bytesRequiredIncludingData);
	// m_data must not change from here on

	for (AssemblyItem const& i: m_items)
		switch (i.type())
		{
		case Operation:
			ret.push_back((byte)i.data());
			break;
		case PushString:
		{
			ret.push_back((byte)Instruction::PUSH32);
			unsigned ii = 0;
			for (auto j: m_strings.at((h256)i.data()))
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
			byte b = max<unsigned>(1, dev::bytesRequired(i.data()));
			ret.push_back((byte)Instruction::PUSH1 - 1 + b);
			ret.resize(ret.size() + b);
			bytesRef byr(&ret.back() + 1 - b, b);
			toBigEndian(i.data(), byr);
			break;
		}
		case PushTag:
		{
			ret.push_back(tagPush);
			tagRef[ret.size()] = (unsigned)i.data();
			ret.resize(ret.size() + bytesPerTag);
			break;
		}
		case PushData: case PushSub:
		{
			ret.push_back(dataRefPush);
			dataRef.insert(make_pair((h256)i.data(), ret.size()));
			ret.resize(ret.size() + bytesPerDataRef);
			break;
		}
		case PushSubSize:
		{
			auto s = m_data[i.data()].size();
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
			tagPos[(unsigned)i.data()] = ret.size();
			ret.push_back((byte)Instruction::JUMPDEST);
			break;
		default:
			BOOST_THROW_EXCEPTION(InvalidOpcode());
		}

	for (auto const& i: tagRef)
	{
		bytesRef r(ret.data() + i.first, bytesPerTag);
		toBigEndian(tagPos[i.second], r);
	}

	if (!m_data.empty())
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
