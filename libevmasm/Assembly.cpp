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
/** @file Assembly.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "Assembly.h"

#include <libevmasm/CommonSubexpressionEliminator.h>
#include <libevmasm/ControlFlowGraph.h>
#include <libevmasm/PeepholeOptimiser.h>
#include <libevmasm/BlockDeduplicator.h>
#include <libevmasm/ConstantOptimiser.h>
#include <libevmasm/GasMeter.h>

#include <fstream>
#include <json/json.h>

using namespace std;
using namespace dev;
using namespace dev::eth;

void Assembly::append(Assembly const& _a)
{
	auto newDeposit = m_deposit + _a.deposit();
	for (AssemblyItem i: _a.m_items)
	{
		if (i.type() == Tag || (i.type() == PushTag && i != errorTag()))
			i.setData(i.data() + m_usedTags);
		else if (i.type() == PushSub || i.type() == PushSubSize)
			i.setData(i.data() + m_subs.size());
		append(i);
	}
	m_deposit = newDeposit;
	m_usedTags += _a.m_usedTags;
	for (auto const& i: _a.m_data)
		m_data.insert(i);
	for (auto const& i: _a.m_strings)
		m_strings.insert(i);
	m_subs += _a.m_subs;
	for (auto const& lib: _a.m_libraries)
		m_libraries.insert(lib);

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

string Assembly::out() const
{
	stringstream ret;
	stream(ret);
	return ret.str();
}

unsigned Assembly::bytesRequired(unsigned subTagSize) const
{
	for (unsigned tagSize = subTagSize; true; ++tagSize)
	{
		unsigned ret = 1;
		for (auto const& i: m_data)
			ret += i.second.size();

		for (AssemblyItem const& i: m_items)
			ret += i.bytesRequired(tagSize);
		if (dev::bytesRequired(ret) <= tagSize)
			return ret;
	}
}

namespace
{

string locationFromSources(StringMap const& _sourceCodes, SourceLocation const& _location)
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

	return cut;
}

class Functionalizer
{
public:
	Functionalizer (ostream& _out, string const& _prefix, StringMap const& _sourceCodes):
		m_out(_out), m_prefix(_prefix), m_sourceCodes(_sourceCodes)
	{}

	void feed(AssemblyItem const& _item)
	{
		if (!_item.location().isEmpty() && _item.location() != m_location)
		{
			flush();
			m_location = _item.location();
			printLocation();
		}
		if (!(
			_item.canBeFunctional() &&
			_item.returnValues() <= 1 &&
			_item.arguments() <= int(m_pending.size())
		))
		{
			flush();
			m_out << m_prefix << (_item.type() == Tag ? "" : "  ") << _item.toAssemblyText() << endl;
			return;
		}
		string expression = _item.toAssemblyText();
		if (_item.arguments() > 0)
		{
			expression += "(";
			for (int i = 0; i < _item.arguments(); ++i)
			{
				expression += m_pending.back();
				m_pending.pop_back();
				if (i + 1 < _item.arguments())
					expression += ", ";
			}
			expression += ")";
		}

		m_pending.push_back(expression);
		if (_item.returnValues() != 1)
			flush();
	}

	void flush()
	{
		for (string const& expression: m_pending)
			m_out << m_prefix << "  " << expression << endl;
		m_pending.clear();
	}

	void printLocation()
	{
		if (!m_location.sourceName && m_location.isEmpty())
			return;
		m_out << m_prefix << "    /*";
		if (m_location.sourceName)
			m_out << " \"" + *m_location.sourceName + "\"";
		if (!m_location.isEmpty())
			m_out << ":" << to_string(m_location.start) + ":" + to_string(m_location.end);
		m_out << "  " << locationFromSources(m_sourceCodes, m_location);
		m_out << " */" << endl;
	}

private:
	strings m_pending;
	SourceLocation m_location;

	ostream& m_out;
	string const& m_prefix;
	StringMap const& m_sourceCodes;
};

}

ostream& Assembly::streamAsm(ostream& _out, string const& _prefix, StringMap const& _sourceCodes) const
{
	Functionalizer f(_out, _prefix, _sourceCodes);

	for (auto const& i: m_items)
		f.feed(i);
	f.flush();

	if (!m_data.empty() || !m_subs.empty())
	{
		_out << _prefix << "stop" << endl;
		for (auto const& i: m_data)
			if (u256(i.first) >= m_subs.size())
				_out << _prefix << "data_" << toHex(u256(i.first)) << " " << toHex(i.second) << endl;

		for (size_t i = 0; i < m_subs.size(); ++i)
		{
			_out << endl << _prefix << "sub_" << i << ": assembly {\n";
			m_subs[i]->streamAsm(_out, _prefix + "    ", _sourceCodes);
			_out << _prefix << "}" << endl;
		}
	}

	return _out;
}

Json::Value Assembly::createJsonValue(string _name, int _begin, int _end, string _value, string _jumpType) const
{
	Json::Value value;
	value["name"] = _name;
	value["begin"] = _begin;
	value["end"] = _end;
	if (!_value.empty())
		value["value"] = _value;
	if (!_jumpType.empty())
		value["jumpType"] = _jumpType;
	return value;
}

string toStringInHex(u256 _value)
{
	std::stringstream hexStr;
	hexStr << hex << _value;
	return hexStr.str();
}

Json::Value Assembly::streamAsmJson(ostream& _out, StringMap const& _sourceCodes) const
{
	Json::Value root;

	Json::Value collection(Json::arrayValue);
	for (AssemblyItem const& i: m_items)
	{
		switch (i.type())
		{
		case Operation:
			collection.append(
				createJsonValue(instructionInfo(i.instruction()).name, i.location().start, i.location().end, i.getJumpTypeAsString()));
			break;
		case Push:
			collection.append(
				createJsonValue("PUSH", i.location().start, i.location().end, toStringInHex(i.data()), i.getJumpTypeAsString()));
			break;
		case PushString:
			collection.append(
				createJsonValue("PUSH tag", i.location().start, i.location().end, m_strings.at((h256)i.data())));
			break;
		case PushTag:
			if (i.data() == 0)
				collection.append(
					createJsonValue("PUSH [ErrorTag]", i.location().start, i.location().end, ""));
			else
				collection.append(
					createJsonValue("PUSH [tag]", i.location().start, i.location().end, string(i.data())));
			break;
		case PushSub:
			collection.append(
				createJsonValue("PUSH [$]", i.location().start, i.location().end, dev::toString(h256(i.data()))));
			break;
		case PushSubSize:
			collection.append(
				createJsonValue("PUSH #[$]", i.location().start, i.location().end, dev::toString(h256(i.data()))));
			break;
		case PushProgramSize:
			collection.append(
				createJsonValue("PUSHSIZE", i.location().start, i.location().end));
			break;
		case PushLibraryAddress:
			collection.append(
				createJsonValue("PUSHLIB", i.location().start, i.location().end, m_libraries.at(h256(i.data())))
			);
			break;
		case Tag:
			collection.append(
				createJsonValue("tag", i.location().start, i.location().end, string(i.data())));
			collection.append(
				createJsonValue("JUMPDEST", i.location().start, i.location().end));
			break;
		case PushData:
			collection.append(createJsonValue("PUSH data", i.location().start, i.location().end, toStringInHex(i.data())));
			break;
		default:
			BOOST_THROW_EXCEPTION(InvalidOpcode());
		}
	}

	root[".code"] = collection;

	if (!m_data.empty() || !m_subs.empty())
	{
		Json::Value data;
		for (auto const& i: m_data)
			if (u256(i.first) >= m_subs.size())
				data[toStringInHex((u256)i.first)] = toHex(i.second);

		for (size_t i = 0; i < m_subs.size(); ++i)
		{
			std::stringstream hexStr;
			hexStr << hex << i;
			data[hexStr.str()] = m_subs[i]->stream(_out, "", _sourceCodes, true);
		}
		root[".data"] = data;
		_out << root;
	}
	return root;
}

Json::Value Assembly::stream(ostream& _out, string const& _prefix, StringMap const& _sourceCodes, bool _inJsonFormat) const
{
	if (_inJsonFormat)
		return streamAsmJson(_out, _sourceCodes);
	else
	{
		streamAsm(_out, _prefix, _sourceCodes);
		return Json::Value();
	}
}

AssemblyItem const& Assembly::append(AssemblyItem const& _i)
{
	m_deposit += _i.deposit();
	m_items.push_back(_i);
	if (m_items.back().location().isEmpty() && !m_currentSourceLocation.isEmpty())
		m_items.back().setLocation(m_currentSourceLocation);
	return back();
}

AssemblyItem Assembly::newPushLibraryAddress(string const& _identifier)
{
	h256 h(dev::keccak256(_identifier));
	m_libraries[h] = _identifier;
	return AssemblyItem(PushLibraryAddress, h);
}

void Assembly::injectStart(AssemblyItem const& _i)
{
	m_items.insert(m_items.begin(), _i);
}

Assembly& Assembly::optimise(bool _enable, bool _isCreation, size_t _runs)
{
	optimiseInternal(_enable, _isCreation, _runs);
	return *this;
}

map<u256, u256> Assembly::optimiseInternal(bool _enable, bool _isCreation, size_t _runs)
{
	for (size_t subId = 0; subId < m_subs.size(); ++subId)
	{
		map<u256, u256> subTagReplacements = m_subs[subId]->optimiseInternal(_enable, false, _runs);
		BlockDeduplicator::applyTagReplacement(m_items, subTagReplacements, subId);
	}

	map<u256, u256> tagReplacements;
	for (unsigned count = 1; count > 0;)
	{
		count = 0;

		PeepholeOptimiser peepOpt(m_items);
		while (peepOpt.optimise())
			count++;

		if (!_enable)
			continue;

		// This only modifies PushTags, we have to run again to actually remove code.
		BlockDeduplicator dedup(m_items);
		if (dedup.deduplicate())
		{
			tagReplacements.insert(dedup.replacedTags().begin(), dedup.replacedTags().end());
			count++;
		}

		{
			// Control flow graph optimization has been here before but is disabled because it
			// assumes we only jump to tags that are pushed. This is not the case anymore with
			// function types that can be stored in storage.
			AssemblyItems optimisedItems;

			auto iter = m_items.begin();
			while (iter != m_items.end())
			{
				KnownState emptyState;
				CommonSubexpressionEliminator eliminator(emptyState);
				auto orig = iter;
				iter = eliminator.feedItems(iter, m_items.end());
				bool shouldReplace = false;
				AssemblyItems optimisedChunk;
				try
				{
					optimisedChunk = eliminator.getOptimizedItems();
					shouldReplace = (optimisedChunk.size() < size_t(iter - orig));
				}
				catch (StackTooDeepException const&)
				{
					// This might happen if the opcode reconstruction is not as efficient
					// as the hand-crafted code.
				}
				catch (ItemNotAvailableException const&)
				{
					// This might happen if e.g. associativity and commutativity rules
					// reorganise the expression tree, but not all leaves are available.
				}

				if (shouldReplace)
				{
					count++;
					optimisedItems += optimisedChunk;
				}
				else
					copy(orig, iter, back_inserter(optimisedItems));
			}
			if (optimisedItems.size() < m_items.size())
			{
				m_items = move(optimisedItems);
				count++;
			}
		}
	}

	if (_enable)
		ConstantOptimisationMethod::optimiseConstants(
			_isCreation,
			_isCreation ? 1 : _runs,
			*this,
			m_items
		);

	return tagReplacements;
}

LinkerObject const& Assembly::assemble() const
{
	if (!m_assembledObject.bytecode.empty())
		return m_assembledObject;

	size_t subTagSize = 1;
	for (auto const& sub: m_subs)
	{
		sub->assemble();
		if (!sub->m_tagPositionsInBytecode.empty())
			subTagSize = max(subTagSize, *max_element(sub->m_tagPositionsInBytecode.begin(), sub->m_tagPositionsInBytecode.end()));
	}

	LinkerObject& ret = m_assembledObject;

	size_t bytesRequiredForCode = bytesRequired(subTagSize);
	m_tagPositionsInBytecode = vector<size_t>(m_usedTags, -1);
	map<size_t, pair<size_t, size_t>> tagRef;
	multimap<h256, unsigned> dataRef;
	multimap<size_t, size_t> subRef;
	vector<unsigned> sizeRef; ///< Pointers to code locations where the size of the program is inserted
	unsigned bytesPerTag = dev::bytesRequired(bytesRequiredForCode);
	byte tagPush = (byte)Instruction::PUSH1 - 1 + bytesPerTag;

	unsigned bytesRequiredIncludingData = bytesRequiredForCode + 1 + m_auxiliaryData.size();
	for (auto const& sub: m_subs)
		bytesRequiredIncludingData += sub->assemble().bytecode.size();

	unsigned bytesPerDataRef = dev::bytesRequired(bytesRequiredIncludingData);
	byte dataRefPush = (byte)Instruction::PUSH1 - 1 + bytesPerDataRef;
	ret.bytecode.reserve(bytesRequiredIncludingData);

	for (AssemblyItem const& i: m_items)
	{
		// store position of the invalid jump destination
		if (i.type() != Tag && m_tagPositionsInBytecode[0] == size_t(-1))
			m_tagPositionsInBytecode[0] = ret.bytecode.size();

		switch (i.type())
		{
		case Operation:
			ret.bytecode.push_back((byte)i.instruction());
			break;
		case PushString:
		{
			ret.bytecode.push_back((byte)Instruction::PUSH32);
			unsigned ii = 0;
			for (auto j: m_strings.at((h256)i.data()))
				if (++ii > 32)
					break;
				else
					ret.bytecode.push_back((byte)j);
			while (ii++ < 32)
				ret.bytecode.push_back(0);
			break;
		}
		case Push:
		{
			byte b = max<unsigned>(1, dev::bytesRequired(i.data()));
			ret.bytecode.push_back((byte)Instruction::PUSH1 - 1 + b);
			ret.bytecode.resize(ret.bytecode.size() + b);
			bytesRef byr(&ret.bytecode.back() + 1 - b, b);
			toBigEndian(i.data(), byr);
			break;
		}
		case PushTag:
		{
			ret.bytecode.push_back(tagPush);
			tagRef[ret.bytecode.size()] = i.splitForeignPushTag();
			ret.bytecode.resize(ret.bytecode.size() + bytesPerTag);
			break;
		}
		case PushData:
			ret.bytecode.push_back(dataRefPush);
			dataRef.insert(make_pair((h256)i.data(), ret.bytecode.size()));
			ret.bytecode.resize(ret.bytecode.size() + bytesPerDataRef);
			break;
		case PushSub:
			ret.bytecode.push_back(dataRefPush);
			subRef.insert(make_pair(size_t(i.data()), ret.bytecode.size()));
			ret.bytecode.resize(ret.bytecode.size() + bytesPerDataRef);
			break;
		case PushSubSize:
		{
			auto s = m_subs.at(size_t(i.data()))->assemble().bytecode.size();
			i.setPushedValue(u256(s));
			byte b = max<unsigned>(1, dev::bytesRequired(s));
			ret.bytecode.push_back((byte)Instruction::PUSH1 - 1 + b);
			ret.bytecode.resize(ret.bytecode.size() + b);
			bytesRef byr(&ret.bytecode.back() + 1 - b, b);
			toBigEndian(s, byr);
			break;
		}
		case PushProgramSize:
		{
			ret.bytecode.push_back(dataRefPush);
			sizeRef.push_back(ret.bytecode.size());
			ret.bytecode.resize(ret.bytecode.size() + bytesPerDataRef);
			break;
		}
		case PushLibraryAddress:
			ret.bytecode.push_back(byte(Instruction::PUSH20));
			ret.linkReferences[ret.bytecode.size()] = m_libraries.at(i.data());
			ret.bytecode.resize(ret.bytecode.size() + 20);
			break;
		case Tag:
			assertThrow(i.data() != 0, AssemblyException, "");
			assertThrow(i.splitForeignPushTag().first == size_t(-1), AssemblyException, "Foreign tag.");
			assertThrow(ret.bytecode.size() < 0xffffffffL, AssemblyException, "Tag too large.");
			m_tagPositionsInBytecode[size_t(i.data())] = ret.bytecode.size();
			ret.bytecode.push_back((byte)Instruction::JUMPDEST);
			break;
		default:
			BOOST_THROW_EXCEPTION(InvalidOpcode());
		}
	}

	if (!m_subs.empty() || !m_data.empty() || !m_auxiliaryData.empty())
		// Append a STOP just to be sure.
		ret.bytecode.push_back(0);

	for (size_t i = 0; i < m_subs.size(); ++i)
	{
		auto references = subRef.equal_range(i);
		if (references.first == references.second)
			continue;
		for (auto ref = references.first; ref != references.second; ++ref)
		{
			bytesRef r(ret.bytecode.data() + ref->second, bytesPerDataRef);
			toBigEndian(ret.bytecode.size(), r);
		}
		ret.append(m_subs[i]->assemble());
	}
	for (auto const& i: tagRef)
	{
		size_t subId;
		size_t tagId;
		tie(subId, tagId) = i.second;
		assertThrow(subId == size_t(-1) || subId < m_subs.size(), AssemblyException, "Invalid sub id");
		std::vector<size_t> const& tagPositions =
			subId == size_t(-1) ?
			m_tagPositionsInBytecode :
			m_subs[subId]->m_tagPositionsInBytecode;
		assertThrow(tagId < tagPositions.size(), AssemblyException, "Reference to non-existing tag.");
		size_t pos = tagPositions[tagId];
		assertThrow(pos != size_t(-1), AssemblyException, "Reference to tag without position.");
		assertThrow(dev::bytesRequired(pos) <= bytesPerTag, AssemblyException, "Tag too large for reserved space.");
		bytesRef r(ret.bytecode.data() + i.first, bytesPerTag);
		toBigEndian(pos, r);
	}
	for (auto const& dataItem: m_data)
	{
		auto references = dataRef.equal_range(dataItem.first);
		if (references.first == references.second)
			continue;
		for (auto ref = references.first; ref != references.second; ++ref)
		{
			bytesRef r(ret.bytecode.data() + ref->second, bytesPerDataRef);
			toBigEndian(ret.bytecode.size(), r);
		}
		ret.bytecode += dataItem.second;
	}

	ret.bytecode += m_auxiliaryData;

	for (unsigned pos: sizeRef)
	{
		bytesRef r(ret.bytecode.data() + pos, bytesPerDataRef);
		toBigEndian(ret.bytecode.size(), r);
	}
	return ret;
}
