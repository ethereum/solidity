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
// SPDX-License-Identifier: GPL-3.0
/** @file Assembly.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include <libevmasm/Assembly.h>

#include <libevmasm/CommonSubexpressionEliminator.h>
#include <libevmasm/ControlFlowGraph.h>
#include <libevmasm/PeepholeOptimiser.h>
#include <libevmasm/Inliner.h>
#include <libevmasm/JumpdestRemover.h>
#include <libevmasm/BlockDeduplicator.h>
#include <libevmasm/ConstantOptimiser.h>
#include <libevmasm/GasMeter.h>

#include <liblangutil/CharStream.h>
#include <liblangutil/Exceptions.h>

#include <json/json.h>

#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/view/enumerate.hpp>

#include <fstream>
#include <limits>

using namespace std;
using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::langutil;
using namespace solidity::util;

AssemblyItem const& Assembly::append(AssemblyItem _i)
{
	assertThrow(m_deposit >= 0, AssemblyException, "Stack underflow.");
	m_deposit += static_cast<int>(_i.deposit());
	auto& currentItems = m_codeSections.at(m_currentCodeSection).items;
	currentItems.emplace_back(std::move(_i));
	if (!currentItems.back().location().isValid() && m_currentSourceLocation.isValid())
		currentItems.back().setLocation(m_currentSourceLocation);
	currentItems.back().m_modifierDepth = m_currentModifierDepth;
	return currentItems.back();
}

unsigned Assembly::codeSize(unsigned subTagSize) const
{
	for (unsigned tagSize = subTagSize; true; ++tagSize)
	{
		size_t ret = 1;

		for (auto const& codeSection: m_codeSections)
			for (AssemblyItem const& i: codeSection.items)
				ret += i.bytesRequired(tagSize, Precision::Approximate);
		if (numberEncodingSize(ret) <= tagSize)
			return static_cast<unsigned>(ret);
	}
}

namespace
{

string locationFromSources(StringMap const& _sourceCodes, SourceLocation const& _location)
{
	if (!_location.hasText() || _sourceCodes.empty())
		return {};

	auto it = _sourceCodes.find(*_location.sourceName);
	if (it == _sourceCodes.end())
		return {};

	return CharStream::singleLineSnippet(it->second, _location);
}

class Functionalizer
{
public:
	Functionalizer (ostream& _out, string const& _prefix, StringMap const& _sourceCodes, Assembly const& _assembly):
		m_out(_out), m_prefix(_prefix), m_sourceCodes(_sourceCodes), m_assembly(_assembly)
	{}

	void feed(AssemblyItem const& _item, DebugInfoSelection const& _debugInfoSelection)
	{
		if (_item.location().isValid() && _item.location() != m_location)
		{
			flush();
			m_location = _item.location();
			printLocation(_debugInfoSelection);
		}

		string expression = _item.toAssemblyText(m_assembly);

		if (!(
			_item.canBeFunctional() &&
			_item.returnValues() <= 1 &&
			_item.arguments() <= m_pending.size()
		))
		{
			flush();
			m_out << m_prefix << (_item.type() == Tag ? "" : "  ") << expression << endl;
			return;
		}
		if (_item.arguments() > 0)
		{
			expression += "(";
			for (size_t i = 0; i < _item.arguments(); ++i)
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

	void printLocation(DebugInfoSelection const& _debugInfoSelection)
	{
		if (!m_location.isValid() || (!_debugInfoSelection.location && !_debugInfoSelection.snippet))
			return;

		m_out << m_prefix << "    /*";

		if (_debugInfoSelection.location)
		{
			if (m_location.sourceName)
				m_out << " " + escapeAndQuoteString(*m_location.sourceName);
			if (m_location.hasText())
				m_out << ":" << to_string(m_location.start) + ":" + to_string(m_location.end);
		}

		if (_debugInfoSelection.snippet)
		{
			if (_debugInfoSelection.location)
				m_out << "  ";

			m_out << locationFromSources(m_sourceCodes, m_location);
		}

		m_out << " */" << endl;
	}

private:
	strings m_pending;
	SourceLocation m_location;

	ostream& m_out;
	string const& m_prefix;
	StringMap const& m_sourceCodes;
	Assembly const& m_assembly;
};

}

void Assembly::assemblyStream(
	ostream& _out,
	DebugInfoSelection const& _debugInfoSelection,
	string const& _prefix,
	StringMap const& _sourceCodes
) const
{
	Functionalizer f(_out, _prefix, _sourceCodes, *this);

	// TODO: support EOF
	for (auto const& i: m_codeSections.front().items)
		f.feed(i, _debugInfoSelection);
	f.flush();

	if (!m_data.empty() || !m_subs.empty())
	{
		_out << _prefix << "stop" << endl;
		for (auto const& i: m_data)
			if (u256(i.first) >= m_subs.size())
				_out << _prefix << "data_" << toHex(u256(i.first)) << " " << util::toHex(i.second) << endl;

		for (size_t i = 0; i < m_subs.size(); ++i)
		{
			_out << endl << _prefix << "sub_" << i << ": assembly {\n";
			m_subs[i]->assemblyStream(_out, _debugInfoSelection, _prefix + "    ", _sourceCodes);
			_out << _prefix << "}" << endl;
		}
	}

	if (m_auxiliaryData.size() > 0)
		_out << endl << _prefix << "auxdata: 0x" << util::toHex(m_auxiliaryData) << endl;
}

string Assembly::assemblyString(
	DebugInfoSelection const& _debugInfoSelection,
	StringMap const& _sourceCodes
) const
{
	ostringstream tmp;
	assemblyStream(tmp, _debugInfoSelection, "", _sourceCodes);
	return tmp.str();
}

Json::Value Assembly::assemblyJSON(map<string, unsigned> const& _sourceIndices, bool _includeSourceList) const
{
	Json::Value root;
	root[".code"] = Json::arrayValue;
	Json::Value& code = root[".code"];
	// TODO: support EOF
	for (AssemblyItem const& item: m_codeSections.front().items)
	{
		int sourceIndex = -1;
		if (item.location().sourceName)
		{
			auto iter = _sourceIndices.find(*item.location().sourceName);
			if (iter != _sourceIndices.end())
				sourceIndex = static_cast<int>(iter->second);
		}

		auto [name, data] = item.nameAndData();
		Json::Value jsonItem;
		jsonItem["name"] = name;
		jsonItem["begin"] = item.location().start;
		jsonItem["end"] = item.location().end;
		if (item.m_modifierDepth != 0)
			jsonItem["modifierDepth"] = static_cast<int>(item.m_modifierDepth);
		std::string jumpType = item.getJumpTypeAsString();
		if (!jumpType.empty())
			jsonItem["jumpType"] = jumpType;
		if (name == "PUSHLIB")
			data = m_libraries.at(h256(data));
		else if (name == "PUSHIMMUTABLE" || name == "ASSIGNIMMUTABLE")
			data = m_immutables.at(h256(data));
		if (!data.empty())
			jsonItem["value"] = data;
		jsonItem["source"] = sourceIndex;
		code.append(std::move(jsonItem));

		if (item.type() == AssemblyItemType::Tag)
		{
			Json::Value jumpdest;
			jumpdest["name"] = "JUMPDEST";
			jumpdest["begin"] = item.location().start;
			jumpdest["end"] = item.location().end;
			jumpdest["source"] = sourceIndex;
			if (item.m_modifierDepth != 0)
				jumpdest["modifierDepth"] = static_cast<int>(item.m_modifierDepth);
			code.append(std::move(jumpdest));
		}
	}
	if (_includeSourceList)
	{
		root["sourceList"] = Json::arrayValue;
		Json::Value& jsonSourceList = root["sourceList"];
		for (auto const& [name, index]: _sourceIndices)
			jsonSourceList[index] = name;
	}

	if (!m_data.empty() || !m_subs.empty())
	{
		root[".data"] = Json::objectValue;
		Json::Value& data = root[".data"];
		for (auto const& i: m_data)
			if (u256(i.first) >= m_subs.size())
				data[util::toHex(toBigEndian((u256)i.first), util::HexPrefix::DontAdd, util::HexCase::Upper)] = util::toHex(i.second);

		for (size_t i = 0; i < m_subs.size(); ++i)
		{
			std::stringstream hexStr;
			hexStr << hex << i;
			data[hexStr.str()] = m_subs[i]->assemblyJSON(_sourceIndices, /*_includeSourceList = */false);
		}
	}

	if (!m_auxiliaryData.empty())
		root[".auxdata"] = util::toHex(m_auxiliaryData);

	return root;
}

AssemblyItem Assembly::namedTag(string const& _name, size_t _params, size_t _returns, optional<uint64_t> _sourceID)
{
	assertThrow(!_name.empty(), AssemblyException, "Empty named tag.");
	if (m_namedTags.count(_name))
	{
		assertThrow(m_namedTags.at(_name).params == _params, AssemblyException, "");
		assertThrow(m_namedTags.at(_name).returns == _returns, AssemblyException, "");
		assertThrow(m_namedTags.at(_name).sourceID == _sourceID, AssemblyException, "");
	}
	else
		m_namedTags[_name] = {static_cast<size_t>(newTag().data()), _sourceID, _params, _returns};
	return AssemblyItem{Tag, m_namedTags.at(_name).id};
}

AssemblyItem Assembly::newPushLibraryAddress(string const& _identifier)
{
	h256 h(util::keccak256(_identifier));
	m_libraries[h] = _identifier;
	return AssemblyItem{PushLibraryAddress, h};
}

AssemblyItem Assembly::newPushImmutable(string const& _identifier)
{
	h256 h(util::keccak256(_identifier));
	m_immutables[h] = _identifier;
	return AssemblyItem{PushImmutable, h};
}

AssemblyItem Assembly::newImmutableAssignment(string const& _identifier)
{
	h256 h(util::keccak256(_identifier));
	m_immutables[h] = _identifier;
	return AssemblyItem{AssignImmutable, h};
}

Assembly& Assembly::optimise(OptimiserSettings const& _settings)
{
	optimiseInternal(_settings, {});
	return *this;
}

map<u256, u256> const& Assembly::optimiseInternal(
	OptimiserSettings const& _settings,
	std::set<size_t> _tagsReferencedFromOutside
)
{
	if (m_tagReplacements)
		return *m_tagReplacements;

	// Run optimisation for sub-assemblies.
	// TODO: verify and double-check this for EOF.
	for (size_t subId = 0; subId < m_subs.size(); ++subId)
	{
		OptimiserSettings settings = _settings;
		Assembly& sub = *m_subs[subId];
		std::set<size_t> referencedTags;
		for (auto& codeSection: m_codeSections)
			referencedTags += JumpdestRemover::referencedTags(codeSection.items, subId);
		map<u256, u256> const& subTagReplacements = sub.optimiseInternal(
			settings,
			referencedTags
		);
		// Apply the replacements (can be empty).
		for (auto& codeSection: m_codeSections)
			BlockDeduplicator::applyTagReplacement(codeSection.items, subTagReplacements, subId);
	}

	map<u256, u256> tagReplacements;
	// Iterate until no new optimisation possibilities are found.
	for (unsigned count = 1; count > 0;)
	{
		count = 0;

		if (_settings.runInliner && !m_eofVersion.has_value())
			Inliner{
				m_codeSections.front().items,
				_tagsReferencedFromOutside,
				_settings.expectedExecutionsPerDeployment,
				isCreation(),
				_settings.evmVersion
			}.optimise();

		if (_settings.runJumpdestRemover)
		{
			// TODO: verify this for EOF.
			for (auto& codeSection: m_codeSections)
			{
				JumpdestRemover jumpdestOpt{codeSection.items};
				if (jumpdestOpt.optimise(_tagsReferencedFromOutside))
					count++;
			}
		}

		if (_settings.runPeephole)
		{
			// TODO: verify this for EOF.
			for (auto& codeSection: m_codeSections)
			{
				PeepholeOptimiser peepOpt{codeSection.items};
				while (peepOpt.optimise())
				{
					count++;
					assertThrow(count < 64000, OptimizerException, "Peephole optimizer seems to be stuck.");
				}
			}
		}

		// This only modifies PushTags, we have to run again to actually remove code.
		if (_settings.runDeduplicate)
			for (auto& section: m_codeSections)
			{
				BlockDeduplicator deduplicator{section.items};
				if (deduplicator.deduplicate())
				{
					for (auto const& replacement: deduplicator.replacedTags())
					{
						assertThrow(
							replacement.first <= numeric_limits<size_t>::max() && replacement.second <= numeric_limits<size_t>::max(),
							OptimizerException,
							"Invalid tag replacement."
						);
						assertThrow(
							!tagReplacements.count(replacement.first),
							OptimizerException,
							"Replacement already known."
						);
						tagReplacements[replacement.first] = replacement.second;
						if (_tagsReferencedFromOutside.erase(static_cast<size_t>(replacement.first)))
							_tagsReferencedFromOutside.insert(static_cast<size_t>(replacement.second));
					}
					count++;
				}
			}

		// TODO: investigate for EOF
		if (_settings.runCSE && !m_eofVersion.has_value())
		{
			// Control flow graph optimization has been here before but is disabled because it
			// assumes we only jump to tags that are pushed. This is not the case anymore with
			// function types that can be stored in storage.
			AssemblyItems optimisedItems;

			auto& items = m_codeSections.front().items;
			bool usesMSize = ranges::any_of(items, [](AssemblyItem const& _i) {
				return _i == AssemblyItem{Instruction::MSIZE} || _i.type() == VerbatimBytecode;
			});

			auto iter = items.begin();
			while (iter != items.end())
			{
				KnownState emptyState;
				CommonSubexpressionEliminator eliminator{emptyState};
				auto orig = iter;
				iter = eliminator.feedItems(iter, items.end(), usesMSize);
				bool shouldReplace = false;
				AssemblyItems optimisedChunk;
				try
				{
					optimisedChunk = eliminator.getOptimizedItems();
					shouldReplace = (optimisedChunk.size() < static_cast<size_t>(iter - orig));
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
			if (optimisedItems.size() < items.size())
			{
				items = std::move(optimisedItems);
				count++;
			}
		}
	}

	if (_settings.runConstantOptimiser)
		ConstantOptimisationMethod::optimiseConstants(
			isCreation(),
			isCreation() ? 1 : _settings.expectedExecutionsPerDeployment,
			_settings.evmVersion,
			*this
		);

	m_tagReplacements = std::move(tagReplacements);
	return *m_tagReplacements;
}

LinkerObject const& Assembly::assemble() const
{
	assertThrow(!m_invalid, AssemblyException, "Attempted to assemble invalid Assembly object.");
	// Return the already assembled object, if present.
	if (!m_assembledObject.bytecode.empty())
		return m_assembledObject;
	// Otherwise ensure the object is actually clear.
	assertThrow(m_assembledObject.linkReferences.empty(), AssemblyException, "Unexpected link references.");

	LinkerObject& ret = m_assembledObject;

	bool const eof = m_eofVersion.has_value();

	size_t subTagSize = 1;
	map<u256, pair<string, vector<size_t>>> immutableReferencesBySub;
	for (auto const& sub: m_subs)
	{
		auto const& linkerObject = sub->assemble();
		if (!linkerObject.immutableReferences.empty())
		{
			assertThrow(
				immutableReferencesBySub.empty(),
				AssemblyException,
				"More than one sub-assembly references immutables."
			);
			immutableReferencesBySub = linkerObject.immutableReferences;
		}
		for (size_t tagPos: sub->m_tagPositionsInBytecode)
			if (tagPos != numeric_limits<size_t>::max() && tagPos > subTagSize)
				subTagSize = tagPos;
	}

	bool setsImmutables = false;
	bool pushesImmutables = false;

	for (auto const& codeSection: m_codeSections)
		for (auto const& i: codeSection.items)
			if (i.type() == AssignImmutable)
			{
				i.setImmutableOccurrences(immutableReferencesBySub[i.data()].second.size());
				setsImmutables = true;
			}
			else if (i.type() == PushImmutable)
				pushesImmutables = true;
	if (setsImmutables || pushesImmutables)
		assertThrow(
			setsImmutables != pushesImmutables,
			AssemblyException,
			"Cannot push and assign immutables in the same assembly subroutine."
		);

	assertThrow(!m_codeSections.empty(), AssemblyException, "Expected at least one code section.");
	assertThrow(eof || m_codeSections.size() == 1, AssemblyException, "Expected exactly one code section in non-EOF code.");
	assertThrow(
		m_codeSections.front().inputs == 0 && m_codeSections.front().outputs == 0, AssemblyException,
		"Expected the first code section to have zero inputs and outputs."
	);

	unsigned bytesRequiredForSubs = 0;
	// TODO: consider fully producing all sub and data refs in this pass already.
	for (auto&& codeSection: m_codeSections)
		for (AssemblyItem const& i: codeSection.items)
			if (i.type() == PushSub)
				bytesRequiredForSubs += static_cast<unsigned>(subAssemblyById(static_cast<size_t>(i.data()))->assemble().bytecode.size());
	unsigned bytesRequiredForDataUpperBound = static_cast<unsigned>(m_auxiliaryData.size());
	// Some of these may be unreferenced and not actually end up in data.
	for (auto const& dataItem: m_data)
		bytesRequiredForDataUpperBound += static_cast<unsigned>(dataItem.second.size());
	unsigned bytesRequiredForDataAndSubsUpperBound = bytesRequiredForDataUpperBound + bytesRequiredForSubs;

	static auto setBigEndian = [](bytes& _dest, size_t _offset, size_t _size, auto _value) {
		assertThrow(numberEncodingSize(_value) <= _size, AssemblyException, "");
		toBigEndian(_value, bytesRef(_dest.data() + _offset, _size));
	};
	static auto appendBigEndian = [](bytes& _dest, size_t _size, auto _value) {
		_dest.resize(_dest.size() + _size);
		setBigEndian(_dest, _dest.size() - _size, _size, _value);
	};
	static auto appendBigEndianUint16 = [](bytes& _dest, auto _value) {
		static_assert(!std::numeric_limits<decltype(_value)>::is_signed, "only unsigned types or bigint supported");
		assertThrow(_value <= 0xFFFF, AssemblyException, "");
		appendBigEndian(_dest, 2, _value);
	};
	vector<size_t> codeSectionSizeOffsets;
	auto setCodeSectionSize = [&](size_t _section, size_t _size) {
		if (eof)
			setBigEndian(ret.bytecode, codeSectionSizeOffsets.at(_section), 2, _size);
	};
	std::optional<size_t> dataSectionSizeOffset;
	auto setDataSectionSize = [&](size_t _size) {
		if (eof)
		{
			assertThrow(dataSectionSizeOffset.has_value(), AssemblyException, "");
			assertThrow(_size <= 0xFFFF, AssemblyException, "Invalid data section size.");
			setBigEndian(ret.bytecode, *dataSectionSizeOffset, 2, _size);
		}
	};
	// Insert EOF1 header.
	if (eof)
	{
		ret.bytecode.push_back(0xef);
		ret.bytecode.push_back(0x00);
		ret.bytecode.push_back(0x01); // version 1

		ret.bytecode.push_back(0x01); // kind=type
		appendBigEndianUint16(ret.bytecode, m_codeSections.size() * 4u); // length of type section

		ret.bytecode.push_back(0x02); // kind=code
		appendBigEndianUint16(ret.bytecode, m_codeSections.size()); // placeholder for number of code sections

		for (auto const& codeSection: m_codeSections)
		{
			(void)codeSection;
			codeSectionSizeOffsets.emplace_back(ret.bytecode.size());
			appendBigEndianUint16(ret.bytecode, 0u); // placeholder for length of code
		}

		ret.bytecode.push_back(0x03); // kind=data
		dataSectionSizeOffset = ret.bytecode.size();
		appendBigEndianUint16(ret.bytecode, 0u); // length of data

		ret.bytecode.push_back(0x00); // terminator

		for (auto const& codeSection: m_codeSections)
		{
			ret.bytecode.push_back(codeSection.inputs);
			ret.bytecode.push_back(codeSection.outputs);
			appendBigEndianUint16(ret.bytecode, codeSection.maxStackHeight);
		}
	}

	unsigned headerSize = static_cast<unsigned>(ret.bytecode.size());
	unsigned bytesRequiredForCode = codeSize(static_cast<unsigned>(subTagSize));
	m_tagPositionsInBytecode = vector<size_t>(m_usedTags, numeric_limits<size_t>::max());
	struct TagRef
	{
		size_t subId = 0;
		size_t tagId = 0;
		bool isRelative = 0;
	};
	map<size_t, TagRef> tagRef;
	multimap<h256, unsigned> dataRef;
	multimap<size_t, size_t> subRef;
	vector<unsigned> sizeRef; ///< Pointers to code locations where the size of the program is inserted
	unsigned bytesPerTag = numberEncodingSize(headerSize + bytesRequiredForCode + bytesRequiredForDataUpperBound);
	uint8_t tagPush = static_cast<uint8_t>(pushInstruction(bytesPerTag));

	if (eof)
	{
		bytesPerTag = 2;
		tagPush = static_cast<uint8_t>(Instruction::INVALID);
	}
	else
		++bytesRequiredForCode; ///< Additional INVALID marker.

	unsigned bytesRequiredIncludingDataAndSubsUpperBound = headerSize + bytesRequiredForCode + bytesRequiredForDataAndSubsUpperBound;
	unsigned bytesPerDataRef = numberEncodingSize(bytesRequiredIncludingDataAndSubsUpperBound);
	uint8_t dataRefPush = static_cast<uint8_t>(pushInstruction(bytesPerDataRef));
	ret.bytecode.reserve(bytesRequiredIncludingDataAndSubsUpperBound);

	for (auto&& [codeSectionIndex, codeSection]: m_codeSections | ranges::views::enumerate)
	{
		auto const sectionStart = ret.bytecode.size();

		for (AssemblyItem const& i: codeSection.items)
		{
			// store position of the invalid jump destination
			if (i.type() != Tag && m_tagPositionsInBytecode[0] == numeric_limits<size_t>::max())
				m_tagPositionsInBytecode[0] = ret.bytecode.size();

			switch (i.type())
			{
			case Operation:
				ret.bytecode.push_back(static_cast<uint8_t>(i.instruction()));
				break;
			case Push:
			{
				unsigned b = max<unsigned>(1, numberEncodingSize(i.data()));
				ret.bytecode.push_back(static_cast<uint8_t>(pushInstruction(b)));
				appendBigEndian(ret.bytecode, b, i.data());
				break;
			}
			case PushTag:
			{
				assertThrow(!eof, AssemblyException, "Push tag in EOF code");
				ret.bytecode.push_back(tagPush);
				auto [subId, tagId] = i.splitForeignPushTag();
				tagRef[ret.bytecode.size()] = TagRef{subId, tagId, false};
				ret.bytecode.resize(ret.bytecode.size() + bytesPerTag);
				break;
			}
			case PushData:
				ret.bytecode.push_back(dataRefPush);
				dataRef.insert(make_pair(h256(i.data()), ret.bytecode.size()));
				ret.bytecode.resize(ret.bytecode.size() + bytesPerDataRef);
				break;
			case PushSub:
				assertThrow(i.data() <= numeric_limits<size_t>::max(), AssemblyException, "");
				ret.bytecode.push_back(dataRefPush);
				subRef.insert(make_pair(static_cast<size_t>(i.data()), ret.bytecode.size()));
				ret.bytecode.resize(ret.bytecode.size() + bytesPerDataRef);
				break;
			case PushSubSize:
			{
				assertThrow(i.data() <= numeric_limits<size_t>::max(), AssemblyException, "");
				auto s = subAssemblyById(static_cast<size_t>(i.data()))->assemble().bytecode.size();
				i.setPushedValue(u256(s));
				unsigned b = max<unsigned>(1, numberEncodingSize(s));
				ret.bytecode.push_back(static_cast<uint8_t>(pushInstruction(b)));
				appendBigEndian(ret.bytecode, b, s);
				break;
			}
			case PushProgramSize:
			{
				ret.bytecode.push_back(dataRefPush);
				sizeRef.push_back(static_cast<unsigned>(ret.bytecode.size()));
				ret.bytecode.resize(ret.bytecode.size() + bytesPerDataRef);
				break;
			}
			case PushLibraryAddress:
				ret.bytecode.push_back(static_cast<uint8_t>(Instruction::PUSH20));
				ret.linkReferences[ret.bytecode.size()] = m_libraries.at(i.data());
				ret.bytecode.resize(ret.bytecode.size() + 20);
				break;
			case PushImmutable:
				ret.bytecode.push_back(static_cast<uint8_t>(Instruction::PUSH32));
				// Maps keccak back to the "identifier" string of that immutable.
				ret.immutableReferences[i.data()].first = m_immutables.at(i.data());
				// Record the bytecode offset of the PUSH32 argument.
				ret.immutableReferences[i.data()].second.emplace_back(ret.bytecode.size());
				// Advance bytecode by 32 bytes (default initialized).
				ret.bytecode.resize(ret.bytecode.size() + 32);
				break;
			case VerbatimBytecode:
				ret.bytecode += i.verbatimData();
				break;
			case AssignImmutable:
			{
				// Expect 2 elements on stack (source, dest_base)
				auto const& offsets = immutableReferencesBySub[i.data()].second;
				for (auto [j, offset]: offsets | ranges::views::enumerate)
				{
					if (j != offsets.size() - 1)
					{
						ret.bytecode.push_back(uint8_t(Instruction::DUP2));
						ret.bytecode.push_back(uint8_t(Instruction::DUP2));
					}
					// TODO: should we make use of the constant optimizer methods for pushing the offsets?
					bytes offsetBytes = toCompactBigEndian(u256(offset));
					ret.bytecode.push_back(static_cast<uint8_t>(pushInstruction(static_cast<unsigned>(offsetBytes.size()))));
					ret.bytecode += offsetBytes;
					ret.bytecode.push_back(uint8_t(Instruction::ADD));
					ret.bytecode.push_back(uint8_t(Instruction::MSTORE));
				}
				if (offsets.empty())
				{
					ret.bytecode.push_back(uint8_t(Instruction::POP));
					ret.bytecode.push_back(uint8_t(Instruction::POP));
				}
				immutableReferencesBySub.erase(i.data());
				break;
			}
			case PushDeployTimeAddress:
				ret.bytecode.push_back(static_cast<uint8_t>(Instruction::PUSH20));
				ret.bytecode.resize(ret.bytecode.size() + 20);
				break;
			case Tag:
			{
				assertThrow(i.data() != 0, AssemblyException, "Invalid tag position.");
				assertThrow(i.splitForeignPushTag().first == numeric_limits<size_t>::max(), AssemblyException, "Foreign tag.");
				size_t tagId = static_cast<size_t>(i.data());
				assertThrow(ret.bytecode.size() < 0xffffffffL, AssemblyException, "Tag too large.");
				assertThrow(m_tagPositionsInBytecode[tagId] == numeric_limits<size_t>::max(), AssemblyException, "Duplicate tag position.");
				m_tagPositionsInBytecode[tagId] = ret.bytecode.size();
				if (!eof)
					ret.bytecode.push_back(static_cast<uint8_t>(Instruction::JUMPDEST));
				break;
			}
			case CallF:
			{
				assertThrow(eof, AssemblyException, "Function call (CALLF) in non-EOF code");
				ret.bytecode.push_back(static_cast<uint8_t>(Instruction::CALLF));
				appendBigEndianUint16(ret.bytecode, i.data());
				break;
			}
			case RetF:
			{
				assertThrow(eof, AssemblyException, "Function return (RETF) in non-EOF code");
				ret.bytecode.push_back(static_cast<uint8_t>(Instruction::RETF));
				break;
			}
			case RelativeJump:
			case ConditionalRelativeJump:
			{
				assertThrow(eof, AssemblyException, "Relative jump in non-EOF code");
				ret.bytecode.push_back(static_cast<uint8_t>(i.type() == RelativeJump ? Instruction::RJUMP : Instruction::RJUMPI));
				auto [subId, tagId] = i.splitForeignPushTag();
				tagRef[ret.bytecode.size()] = TagRef{subId, tagId, true};
				appendBigEndianUint16(ret.bytecode, 0u);
				break;
			}
			default:
				assertThrow(false, InvalidOpcode, "Unexpected opcode while assembling.");
			}
		}

		auto sectionEnd = ret.bytecode.size();

		setCodeSectionSize(codeSectionIndex, sectionEnd - sectionStart);
	}

	if (!immutableReferencesBySub.empty())
		throw
			langutil::Error(
				1284_error,
				langutil::Error::Type::CodeGenerationError,
				"Some immutables were read from but never assigned, possibly because of optimization."
			);

	if (!eof && (!m_subs.empty() || !m_data.empty() || !m_auxiliaryData.empty()))
		// Append an INVALID here to help tests find miscompilation.
		ret.bytecode.push_back(static_cast<uint8_t>(Instruction::INVALID));

	auto const dataStart = ret.bytecode.size();

	// TODO: this should better be done separately on develop.
	{
		map<h256, size_t> subAssemblyOffsets;
		for (auto const& [subIdPath, bytecodeOffset]: subRef)
		{
			LinkerObject subObject = subAssemblyById(subIdPath)->assemble();
			util::h256 h(util::keccak256(util::asString(subObject.bytecode)));
			bytesRef r(ret.bytecode.data() + bytecodeOffset, bytesPerDataRef);
			if (size_t* subAssemblyOffset = util::valueOrNullptr(subAssemblyOffsets, h))
				toBigEndian(*subAssemblyOffset, r);
			else
			{
				toBigEndian(ret.bytecode.size(), r);
				subAssemblyOffsets[h] = ret.bytecode.size();
				ret.bytecode += subObject.bytecode;
			}
			// TODO: double-check this.
			for (auto const& ref: subObject.linkReferences)
				ret.linkReferences[ref.first + subAssemblyOffsets[h]] = ref.second;
		}
	}

	for (auto const& [bytecodeOffset, ref]: tagRef)
	{
		size_t subId = ref.subId;
		size_t tagId = ref.tagId;
		bool relative = ref.isRelative;
		assertThrow(subId == numeric_limits<size_t>::max() || subId < m_subs.size(), AssemblyException, "Invalid sub id");
		vector<size_t> const& tagPositions =
			subId == numeric_limits<size_t>::max() ?
			m_tagPositionsInBytecode :
			m_subs[subId]->m_tagPositionsInBytecode;
		assertThrow(tagId < tagPositions.size(), AssemblyException, "Reference to non-existing tag.");
		size_t pos = tagPositions[tagId];
		assertThrow(pos != numeric_limits<size_t>::max(), AssemblyException, "Reference to tag without position.");
		assertThrow(numberEncodingSize(pos) <= bytesPerTag, AssemblyException, "Tag too large for reserved space.");
		if (relative)
		{
			assertThrow(m_eofVersion.has_value(), AssemblyException, "Relative jump outside EOF");
			assertThrow(subId == numeric_limits<size_t>::max(), AssemblyException, "Relative jump to sub");
			assertThrow(
				static_cast<ptrdiff_t>(pos) - static_cast<ptrdiff_t>(bytecodeOffset + 2u) < 0x7FFF &&
				static_cast<ptrdiff_t>(pos) - static_cast<ptrdiff_t>(bytecodeOffset + 2u) >= -0x8000,
				AssemblyException,
				"Relative jump too far"
			);
			toBigEndian(pos - (bytecodeOffset + 2u), bytesRef(ret.bytecode.data() + bytecodeOffset, 2));
		}
		else
		{
			assertThrow(!m_eofVersion.has_value(), AssemblyException, "Dynamic tag reference within EOF");
			toBigEndian(pos, bytesRef(ret.bytecode.data() + bytecodeOffset, bytesPerTag));
		}
	}
	for (auto const& [name, tagInfo]: m_namedTags)
	{
		size_t position = m_tagPositionsInBytecode.at(tagInfo.id);
		optional<size_t> tagIndex;
		for (auto& codeSection: m_codeSections)
			for (auto&& [index, item]: codeSection.items | ranges::views::enumerate)
				if (item.type() == Tag && static_cast<size_t>(item.data()) == tagInfo.id)
				{
					tagIndex = index;
					break;
				}
		ret.functionDebugData[name] = {
			position == numeric_limits<size_t>::max() ? nullopt : optional<size_t>{position},
			tagIndex,
			tagInfo.sourceID,
			tagInfo.params,
			tagInfo.returns
		};
	}

	for (auto const& dataItem: m_data)
	{
		auto references = dataRef.equal_range(dataItem.first);
		if (references.first == references.second)
			continue;
		for (auto ref = references.first; ref != references.second; ++ref)
			toBigEndian(ret.bytecode.size(), bytesRef(ret.bytecode.data() + ref->second, bytesPerDataRef));
		ret.bytecode += dataItem.second;
	}

	ret.bytecode += m_auxiliaryData;

	for (unsigned pos: sizeRef)
		setBigEndian(ret.bytecode, pos, bytesPerDataRef, ret.bytecode.size());

	auto dataLength = ret.bytecode.size() - dataStart;
	assertThrow(
		bytesRequiredForDataAndSubsUpperBound >= dataLength,
		AssemblyException,
		"More data than expected. " + to_string(dataLength) + " > " + to_string(bytesRequiredForDataUpperBound)
	);
	setDataSectionSize(dataLength);

	return ret;
}

vector<size_t> Assembly::decodeSubPath(size_t _subObjectId) const
{
	if (_subObjectId < m_subs.size())
		return {_subObjectId};

	auto subIdPathIt = find_if(
		m_subPaths.begin(),
		m_subPaths.end(),
		[_subObjectId](auto const& subId) { return subId.second == _subObjectId; }
	);

	assertThrow(subIdPathIt != m_subPaths.end(), AssemblyException, "");
	return subIdPathIt->first;
}

size_t Assembly::encodeSubPath(vector<size_t> const& _subPath)
{
	assertThrow(!_subPath.empty(), AssemblyException, "");
	if (_subPath.size() == 1)
	{
		assertThrow(_subPath[0] < m_subs.size(), AssemblyException, "");
		return _subPath[0];
	}

	if (m_subPaths.find(_subPath) == m_subPaths.end())
	{
		size_t objectId = numeric_limits<size_t>::max() - m_subPaths.size();
		assertThrow(objectId >= m_subs.size(), AssemblyException, "");
		m_subPaths[_subPath] = objectId;
	}

	return m_subPaths[_subPath];
}

Assembly const* Assembly::subAssemblyById(size_t _subId) const
{
	vector<size_t> subIds = decodeSubPath(_subId);
	Assembly const* currentAssembly = this;
	for (size_t currentSubId: subIds)
	{
		currentAssembly = currentAssembly->m_subs.at(currentSubId).get();
		assertThrow(currentAssembly, AssemblyException, "");
	}

	assertThrow(currentAssembly != this, AssemblyException, "");
	return currentAssembly;
}

Assembly::OptimiserSettings Assembly::OptimiserSettings::translateSettings(frontend::OptimiserSettings const& _settings, langutil::EVMVersion const& _evmVersion)
{
	// Constructing it this way so that we notice changes in the fields.
	evmasm::Assembly::OptimiserSettings asmSettings{false,  false, false, false, false, false, _evmVersion, 0};
	asmSettings.runInliner = _settings.runInliner;
	asmSettings.runJumpdestRemover = _settings.runJumpdestRemover;
	asmSettings.runPeephole = _settings.runPeephole;
	asmSettings.runDeduplicate = _settings.runDeduplicate;
	asmSettings.runCSE = _settings.runCSE;
	asmSettings.runConstantOptimiser = _settings.runConstantOptimiser;
	asmSettings.expectedExecutionsPerDeployment = _settings.expectedExecutionsPerDeployment;
	asmSettings.evmVersion = _evmVersion;
	return asmSettings;
}
