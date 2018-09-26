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
#include <libevmasm/JumpdestRemover.h>
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
		if (i.type() == Tag || i.type() == PushTag)
			i.setData(i.data() + m_usedTags);
		else if (i.type() == PushSub || i.type() == PushSubSize)
			i.setData(i.data() + m_subs.size());
		append(i);
	}
	m_deposit = newDeposit;
	m_usedTags += _a.m_usedTags;
	// This does not transfer the names of named tags on purpose. The tags themselves are
	// transferred, but their names are only available inside the assembly.
	for (auto const& i: _a.m_data)
		m_data.insert(i);
	for (auto const& i: _a.m_strings)
		m_strings.insert(i);
	m_subs += _a.m_subs;
	for (auto const& lib: _a.m_libraries)
		m_libraries.insert(lib);
}

void Assembly::append(Assembly const& _a, int _deposit)
{
	assertThrow(_deposit <= _a.m_deposit, InvalidDeposit, "");

	append(_a);
	while (_deposit++ < _a.m_deposit)
		append(Instruction::POP);
}

AssemblyItem const& Assembly::append(AssemblyItem const& _i)
{
	assertThrow(m_deposit >= 0, AssemblyException, "Stack underflow.");
	m_deposit += _i.deposit();
	m_items.push_back(_i);
	if (m_items.back().location().isEmpty() && !m_currentSourceLocation.isEmpty())
		m_items.back().setLocation(m_currentSourceLocation);
	return back();
}

void Assembly::injectStart(AssemblyItem const& _i)
{
	m_items.insert(m_items.begin(), _i);
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

void Assembly::assemblyStream(ostream& _out, string const& _prefix, StringMap const& _sourceCodes) const
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
			m_subs[i]->assemblyStream(_out, _prefix + "    ", _sourceCodes);
			_out << _prefix << "}" << endl;
		}
	}

	if (m_auxiliaryData.size() > 0)
		_out << endl << _prefix << "auxdata: 0x" << toHex(m_auxiliaryData) << endl;
}

string Assembly::assemblyString(StringMap const& _sourceCodes) const
{
	ostringstream tmp;
	assemblyStream(tmp, "", _sourceCodes);
	return tmp.str();
}

Json::Value Assembly::createJsonValue(string _name, int _begin, int _end, string _value, string _jumpType)
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

string Assembly::toStringInHex(u256 _value)
{
	std::stringstream hexStr;
	hexStr << hex << _value;
	return hexStr.str();
}

Json::Value Assembly::assemblyJSON(StringMap const& _sourceCodes) const
{
	Json::Value root;

	Json::Value& collection = root[".code"] = Json::arrayValue;
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
					createJsonValue("PUSH [tag]", i.location().start, i.location().end, dev::toString(i.data())));
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
		case PushDeployTimeAddress:
			collection.append(
				createJsonValue("PUSHDEPLOYADDRESS", i.location().start, i.location().end)
			);
			break;
		case Tag:
			collection.append(
				createJsonValue("tag", i.location().start, i.location().end, dev::toString(i.data())));
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

	if (!m_data.empty() || !m_subs.empty())
	{
		Json::Value& data = root[".data"] = Json::objectValue;
		for (auto const& i: m_data)
			if (u256(i.first) >= m_subs.size())
				data[toStringInHex((u256)i.first)] = toHex(i.second);

		for (size_t i = 0; i < m_subs.size(); ++i)
		{
			std::stringstream hexStr;
			hexStr << hex << i;
			data[hexStr.str()] = m_subs[i]->assemblyJSON(_sourceCodes);
		}
	}

	if (m_auxiliaryData.size() > 0)
		root[".auxdata"] = toHex(m_auxiliaryData);

	return root;
}

AssemblyItem Assembly::namedTag(string const& _name)
{
	assertThrow(!_name.empty(), AssemblyException, "Empty named tag.");
	if (!m_namedTags.count(_name))
		m_namedTags[_name] = size_t(newTag().data());
	return AssemblyItem(Tag, m_namedTags.at(_name));
}

AssemblyItem Assembly::newPushLibraryAddress(string const& _identifier)
{
	h256 h(dev::keccak256(_identifier));
	m_libraries[h] = _identifier;
	return AssemblyItem(PushLibraryAddress, h);
}

Assembly& Assembly::optimise(bool _enable, EVMVersion _evmVersion, bool _isCreation, size_t _runs)
{
	OptimiserSettings settings;
	settings.isCreation = _isCreation;
	settings.runJumpdestRemover = true;
	settings.runPeephole = true;
	if (_enable)
	{
		settings.runDeduplicate = true;
		settings.runCSE = true;
		settings.runConstantOptimiser = true;
	}
	settings.evmVersion = _evmVersion;
	settings.expectedExecutionsPerDeployment = _runs;
	optimise(settings);
	return *this;
}


Assembly& Assembly::optimise(OptimiserSettings const& _settings)
{
	optimiseInternal(_settings, {});
	return *this;
}

map<u256, u256> Assembly::optimiseInternal(
	OptimiserSettings const& _settings,
	std::set<size_t> const& _tagsReferencedFromOutside
)
{
	// Run optimisation for sub-assemblies.
	for (size_t subId = 0; subId < m_subs.size(); ++subId)
	{
		OptimiserSettings settings = _settings;
		// Disable creation mode for sub-assemblies.
		settings.isCreation = false;
		map<u256, u256> subTagReplacements = m_subs[subId]->optimiseInternal(
			settings,
			JumpdestRemover::referencedTags(m_items, subId)
		);
		// Apply the replacements (can be empty).
		BlockDeduplicator::applyTagReplacement(m_items, subTagReplacements, subId);
	}

	map<u256, u256> tagReplacements;
	// Iterate until no new optimisation possibilities are found.
	for (unsigned count = 1; count > 0;)
	{
		count = 0;

		if (_settings.runJumpdestRemover)
		{
			JumpdestRemover jumpdestOpt(m_items);
			if (jumpdestOpt.optimise(_tagsReferencedFromOutside))
				count++;
		}

		if (_settings.runPeephole)
		{
			PeepholeOptimiser peepOpt(m_items);
			while (peepOpt.optimise())
			{
				count++;
				assertThrow(count < 64000, OptimizerException, "Peephole optimizer seems to be stuck.");
			}
		}

		// This only modifies PushTags, we have to run again to actually remove code.
		if (_settings.runDeduplicate)
		{
			BlockDeduplicator dedup(m_items);
			if (dedup.deduplicate())
			{
				tagReplacements.insert(dedup.replacedTags().begin(), dedup.replacedTags().end());
				count++;
			}
		}

		if (_settings.runCSE)
		{
			// Control flow graph optimization has been here before but is disabled because it
			// assumes we only jump to tags that are pushed. This is not the case anymore with
			// function types that can be stored in storage.
			AssemblyItems optimisedItems;

			bool usesMSize = (find(m_items.begin(), m_items.end(), AssemblyItem(Instruction::MSIZE)) != m_items.end());

			auto iter = m_items.begin();
			while (iter != m_items.end())
			{
				KnownState emptyState;
				CommonSubexpressionEliminator eliminator(emptyState);
				auto orig = iter;
				iter = eliminator.feedItems(iter, m_items.end(), usesMSize);
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

	if (_settings.runConstantOptimiser)
		ConstantOptimisationMethod::optimiseConstants(
			_settings.isCreation,
			_settings.isCreation ? 1 : _settings.expectedExecutionsPerDeployment,
			_settings.evmVersion,
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
		for (size_t tagPos: sub->m_tagPositionsInBytecode)
			if (tagPos != size_t(-1) && tagPos > subTagSize)
				subTagSize = tagPos;
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
		case PushDeployTimeAddress:
			ret.bytecode.push_back(byte(Instruction::PUSH20));
			ret.bytecode.resize(ret.bytecode.size() + 20);
			break;
		case Tag:
			assertThrow(i.data() != 0, AssemblyException, "Invalid tag position.");
			assertThrow(i.splitForeignPushTag().first == size_t(-1), AssemblyException, "Foreign tag.");
			assertThrow(ret.bytecode.size() < 0xffffffffL, AssemblyException, "Tag too large.");
			assertThrow(m_tagPositionsInBytecode[size_t(i.data())] == size_t(-1), AssemblyException, "Duplicate tag position.");
			m_tagPositionsInBytecode[size_t(i.data())] = ret.bytecode.size();
			ret.bytecode.push_back((byte)Instruction::JUMPDEST);
			break;
		default:
			BOOST_THROW_EXCEPTION(InvalidOpcode());
		}
	}

	if (!m_subs.empty() || !m_data.empty() || !m_auxiliaryData.empty())
		// Append an INVALID here to help tests find miscompilation.
		ret.bytecode.push_back(byte(Instruction::INVALID));

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
