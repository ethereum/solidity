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

#include <libsolutil/JSON.h>
#include <libsolutil/StringUtils.h>

#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/view/enumerate.hpp>

#include <fstream>
#include <limits>
#include <iterator>

using namespace std;
using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::langutil;
using namespace solidity::util;

AssemblyItem const& Assembly::append(AssemblyItem _i)
{
	assertThrow(m_deposit >= 0, AssemblyException, "Stack underflow.");
	m_deposit += static_cast<int>(_i.deposit());
	m_items.emplace_back(std::move(_i));
	if (!m_items.back().location().isValid() && m_currentSourceLocation.isValid())
		m_items.back().setLocation(m_currentSourceLocation);
	m_items.back().m_modifierDepth = m_currentModifierDepth;
	return m_items.back();
}

unsigned Assembly::codeSize(unsigned subTagSize) const
{
	for (unsigned tagSize = subTagSize; true; ++tagSize)
	{
		size_t ret = 1;
		for (auto const& i: m_data)
			ret += i.second.size();

		for (AssemblyItem const& i: m_items)
			ret += i.bytesRequired(tagSize, Precision::Approximate);
		if (numberEncodingSize(ret) <= tagSize)
			return static_cast<unsigned>(ret);
	}
}

void Assembly::importAssemblyItemsFromJSON(Json::Value const& _code)
{
	solAssert(m_items.empty());
	solRequire(_code.isArray(), AssemblyImportException, "Supplied JSON is not an array.");
	for (auto current = begin(_code); current != end(_code); ++current)
	{
		auto const& item = m_items.emplace_back(createAssemblyItemFromJSON(*current, m_sourceList));
		if (item == Instruction::JUMPDEST)
			solThrow(AssemblyImportException, "JUMPDEST instruction without a tag");
		else if (item.type() == AssemblyItemType::Tag)
		{
			++current;
			if (current != end(_code) && createAssemblyItemFromJSON(*current, m_sourceList) != Instruction::JUMPDEST)
				solThrow(AssemblyImportException, "JUMPDEST expected after tag.");
		}
	}
}

AssemblyItem Assembly::createAssemblyItemFromJSON(Json::Value const& _json, std::vector<std::string> const& _sourceList)
{
	solRequire(_json.isObject(), AssemblyImportException, "Supplied JSON is not an object.");
	static set<string> const validMembers{"name", "begin", "end", "source", "value", "modifierDepth", "jumpType"};
	for (auto const& member: _json.getMemberNames())
		solRequire(
			validMembers.count(member),
			AssemblyImportException,
			"Unknown member '" + member + "'. Valid members are " +
			solidity::util::joinHumanReadable(validMembers, ", ") + "."
		);
	solRequire(isOfType<string>(_json["name"]), AssemblyImportException, "Member 'name' missing or not of type string.");
	solRequire(isOfTypeIfExists<int>(_json, "begin"), AssemblyImportException, "Optional member 'begin' not of type int.");
	solRequire(isOfTypeIfExists<int>(_json, "end"), AssemblyImportException, "Optional member 'end' not of type int.");
	solRequire(isOfTypeIfExists<int>(_json, "source"), AssemblyImportException, "Optional member 'source' not of type int.");
	solRequire(isOfTypeIfExists<string>(_json, "value"), AssemblyImportException, "Optional member 'value' not of type string.");
	solRequire(
		isOfTypeIfExists<int>(_json, "modifierDepth"),
		AssemblyImportException,
		"Optional member 'modifierDepth' not of type int."
	);
	solRequire(
		isOfTypeIfExists<string>(_json, "jumpType"),
		AssemblyImportException,
		"Optional member 'jumpType' not of type string."
	);

	string name = get<string>(_json["name"]);
	solRequire(!name.empty(), AssemblyImportException, "Member 'name' was empty.");

	SourceLocation location;
	location.start = get<int>(_json["begin"]);
	location.end = get<int>(_json["end"]);
	int srcIndex = getOrDefault<int>(_json["source"], -1);
	size_t modifierDepth = static_cast<size_t>(getOrDefault<int>(_json["modifierDepth"], 0));
	string value = getOrDefault<string>(_json["value"], "");
	string jumpType = getOrDefault<string>(_json["jumpType"], "");

	auto updateUsedTags = [&](u256 const& data)
	{
		m_usedTags = max(m_usedTags, static_cast<unsigned>(data) + 1);
		return data;
	};

	auto storeImmutableHash = [&](string const& _immutableName) -> h256
	{
		h256 hash(util::keccak256(_immutableName));
		solAssert(m_immutables.count(hash) == 0 || m_immutables[hash] == _immutableName);
		m_immutables[hash] = _immutableName;
		return hash;
	};

	auto storeLibraryHash = [&](string const& _libraryName) -> h256
	{
		h256 hash(util::keccak256(_libraryName));
		solAssert(m_libraries.count(hash) == 0 || m_libraries[hash] == _libraryName);
		m_libraries[hash] = _libraryName;
		return hash;
	};

	auto requireValueDefinedForInstruction = [&](string const& _name, string const& _value)
	{
		solRequire(
			!_value.empty(),
			AssemblyImportException,
			"Member 'value' was not defined for instruction '" + _name + "', but the instruction needs a value."
		);
	};

	auto requireValueUndefinedForInstruction = [&](string const& _name, string const& _value)
	{
		solRequire(
			_value.empty(),
			AssemblyImportException,
			"Member 'value' defined for instruction '" + _name + "', but the instruction does not need a value."
		);
	};

	solRequire(srcIndex >= -1 && srcIndex < static_cast<int>(_sourceList.size()), AssemblyImportException, "srcIndex out of bound.");
	if (srcIndex != -1)
		location.sourceName = std::make_shared<std::string>(_sourceList[static_cast<size_t>(srcIndex)]);

	AssemblyItem result(0);

	if (c_instructions.count(name))
	{
		AssemblyItem item{c_instructions.at(name), location};
		if (!jumpType.empty())
		{
			if (item.instruction() == Instruction::JUMP || item.instruction() == Instruction::JUMPI)
				item.setJumpType(jumpType);
			else
				solThrow(
					AssemblyImportException,
					"Member 'jumpType' set on instruction different from JUMP or JUMPI (was set on instruction '" + name + "')"
				);
		}
		InstructionInfo info = instructionInfo(item.instruction(), m_evmVersion);
		if (info.args == 0)
			requireValueUndefinedForInstruction(name, value);
		result = item;
	}
	else
	{
		solRequire(
			jumpType.empty(),
			AssemblyImportException,
			"Member 'jumpType' set on instruction different from JUMP or JUMPI (was set on instruction '" + name + "')"
		);
		if (name == "PUSH")
		{
			requireValueDefinedForInstruction(name, value);
			result = {AssemblyItemType::Push, u256("0x" + value)};
		}
		else if (name == "PUSH [ErrorTag]")
		{
			requireValueUndefinedForInstruction(name, value);
			result = {AssemblyItemType::PushTag, 0};
		}
		else if (name == "PUSH [tag]")
		{
			requireValueDefinedForInstruction(name, value);
			result = {AssemblyItemType::PushTag, updateUsedTags(u256(value))};
		}
		else if (name == "PUSH [$]")
		{
			requireValueDefinedForInstruction(name, value);
			result = {AssemblyItemType::PushSub, u256("0x" + value)};
		}
		else if (name == "PUSH #[$]")
		{
			requireValueDefinedForInstruction(name, value);
			result = {AssemblyItemType::PushSubSize, u256("0x" + value)};
		}
		else if (name == "PUSHSIZE")
		{
			requireValueUndefinedForInstruction(name, value);
			result = {AssemblyItemType::PushProgramSize, 0};
		}
		else if (name == "PUSHLIB")
		{
			requireValueDefinedForInstruction(name, value);
			result = {AssemblyItemType::PushLibraryAddress, storeLibraryHash(value)};
		}
		else if (name == "PUSHDEPLOYADDRESS")
		{
			requireValueUndefinedForInstruction(name, value);
			result = {AssemblyItemType::PushDeployTimeAddress, 0};
		}
		else if (name == "PUSHIMMUTABLE")
		{
			requireValueDefinedForInstruction(name, value);
			result = {AssemblyItemType::PushImmutable, storeImmutableHash(value)};
		}
		else if (name == "ASSIGNIMMUTABLE")
		{
			requireValueDefinedForInstruction(name, value);
			result = {AssemblyItemType::AssignImmutable, storeImmutableHash(value)};
		}
		else if (name == "tag")
		{
			requireValueDefinedForInstruction(name, value);
			result = {AssemblyItemType::Tag, updateUsedTags(u256(value))};
		}
		else if (name == "PUSH data")
		{
			requireValueDefinedForInstruction(name, value);
			result = {AssemblyItemType::PushData, u256("0x" + value)};
		}
		else if (name == "VERBATIM")
		{
			requireValueUndefinedForInstruction(name, value);
			AssemblyItem item(fromHex(value), 0, 0);
			result = item;
		}
		else
			solThrow(InvalidOpcode, "Invalid opcode: " + name);
	}
	result.setLocation(location);
	result.m_modifierDepth = modifierDepth;
	return result;
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

	for (auto const& i: m_items)
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

Json::Value Assembly::assemblyJSON(bool _includeSourceList) const
{
	Json::Value root;
	root[".code"] = Json::arrayValue;
	Json::Value& code = root[".code"];
	for (AssemblyItem const& item: m_items)
	{
		int sourceIndex = -1;
		if (item.location().sourceName)
		{
			for (size_t index = 0; index < m_sourceList.size(); ++index)
				if (m_sourceList[index] == *item.location().sourceName)
					sourceIndex = static_cast<int>(index);
		}

		auto [name, data] = item.nameAndData(m_evmVersion);
		Json::Value jsonItem;
		jsonItem["name"] = name;
		jsonItem["begin"] = item.location().start;
		jsonItem["end"] = item.location().end;
		if (item.m_modifierDepth != 0)
			jsonItem["modifierDepth"] = static_cast<int>(item.m_modifierDepth);
		string jumpType = item.getJumpTypeAsString();
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
		for (int index = 0; index < static_cast<int>(m_sourceList.size()); ++index)
			jsonSourceList[index] = m_sourceList[static_cast<size_t>(index)];
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
			stringstream hexStr;
			hexStr << hex << i;
			m_subs[i]->setSourceList(m_sourceList);
			data[hexStr.str()] = m_subs[i]->assemblyJSON(false);
		}
	}

	if (!m_auxiliaryData.empty())
		root[".auxdata"] = util::toHex(m_auxiliaryData);

	return root;
}

shared_ptr<Assembly> Assembly::fromJSON(Json::Value const& _json, vector<string> const& _sourceList, int _level)
{
	solRequire(_json.isObject(), AssemblyImportException, "Supplied JSON is not an object.");
	static set<string> const validMembers{".code", ".data", ".auxdata", "sourceList"};
	for (auto const& attribute: _json.getMemberNames())
		solRequire(validMembers.count(attribute), AssemblyImportException, "Unknown attribute '" + attribute + "'.");
	solRequire(_json.isMember(".code"), AssemblyImportException, "Member '.code' does not exist.");
	solRequire(_json[".code"].isArray(), AssemblyImportException, "Member '.code' is not an array.");
	for (auto const& codeItem: _json[".code"])
		solRequire(codeItem.isObject(), AssemblyImportException, "Item of '.code' array is not an object.");

	if (_level == 0)
	{
		if (_json.isMember("sourceList"))
		{
			solRequire(_json["sourceList"].isArray(), AssemblyImportException, "Optional member 'sourceList' is not an array.");
			for (auto const& sourceListItem: _json["sourceList"])
				solRequire(sourceListItem.isString(), AssemblyImportException, "Item of 'sourceList' array is not of type string.");
		}
	} else
		solRequire(
			!_json.isMember("sourceList"),
			AssemblyImportException,
			"Member 'sourceList' is only allowed in root JSON object."
		);

	shared_ptr<Assembly> result = make_shared<Assembly>(langutil::EVMVersion(), _level == 0, "");
	if (_json.isMember("sourceList"))
	{
		solAssert(_level == 0);
		for (auto const& it: _json["sourceList"])
			result->m_sourceList.emplace_back(it.asString());
	} else
		result->m_sourceList = _sourceList;

	result->importAssemblyItemsFromJSON(_json[".code"]);
	if (_json[".auxdata"])
	{
		solRequire(_json[".auxdata"].isString(), AssemblyImportException, "Optional member '.auxdata' is not of type string.");
		bytes auxdata{fromHex(_json[".auxdata"].asString())};
		solRequire(!auxdata.empty(), AssemblyImportException, "Optional member '.auxdata' is not a valid hexadecimal string.");
		result->m_auxiliaryData = auxdata;
	}

	if (_json.isMember(".data"))
	{
		solRequire(_json[".data"].isObject(), AssemblyImportException, "Optional member '.data' is not an object.");
		Json::Value const& data = _json[".data"];
		for (Json::ValueConstIterator dataIter = data.begin(); dataIter != data.end(); dataIter++)
		{
			solRequire(dataIter.key().isString(), AssemblyImportException, "Key inside '.data' is not of type string.");
			string dataItemID = dataIter.key().asString();
			Json::Value const& code = data[dataItemID];
			if (code.isString())
			{
				bytes data_value{fromHex(code.asString())};
				solRequire(
					!data_value.empty(),
					AssemblyImportException,
					"Member '.data' contains a value for '" + dataItemID + "' that is not a valid hexadecimal string.");
				result->m_data[h256(fromHex(dataItemID))] = fromHex(code.asString());
			}
			else if (code.isObject())
			{
				shared_ptr<Assembly> subassembly(Assembly::fromJSON(code, result->m_sourceList, _level + 1));
				solAssert(subassembly);
				result->m_subs.emplace_back(make_shared<Assembly>(*subassembly));
				// TODO: this shouldn't be enough for the general case.
				result->encodeSubPath({0, 0});
			}
			else
				solThrow(AssemblyImportException, "Key inside '.data' '" + dataItemID + "' can only be a valid hex-string or an object.");
		}
	}
	return result;
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
	set<size_t> _tagsReferencedFromOutside
)
{
	if (m_tagReplacements)
		return *m_tagReplacements;

	// Run optimisation for sub-assemblies.
	for (size_t subId = 0; subId < m_subs.size(); ++subId)
	{
		OptimiserSettings settings = _settings;
		Assembly& sub = *m_subs[subId];
		map<u256, u256> const& subTagReplacements = sub.optimiseInternal(
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

		if (_settings.runInliner)
			Inliner{
				m_items,
				_tagsReferencedFromOutside,
				_settings.expectedExecutionsPerDeployment,
				isCreation(),
				_settings.evmVersion
			}.optimise();

		if (_settings.runJumpdestRemover)
		{
			JumpdestRemover jumpdestOpt{m_items};
			if (jumpdestOpt.optimise(_tagsReferencedFromOutside))
				count++;
		}

		if (_settings.runPeephole)
		{
			PeepholeOptimiser peepOpt{m_items};
			while (peepOpt.optimise())
			{
				count++;
				assertThrow(count < 64000, OptimizerException, "Peephole optimizer seems to be stuck.");
			}
		}

		// This only modifies PushTags, we have to run again to actually remove code.
		if (_settings.runDeduplicate)
		{
			BlockDeduplicator deduplicator{m_items};
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

		if (_settings.runCSE)
		{
			// Control flow graph optimization has been here before but is disabled because it
			// assumes we only jump to tags that are pushed. This is not the case anymore with
			// function types that can be stored in storage.
			AssemblyItems optimisedItems;

			bool usesMSize = ranges::any_of(m_items, [](AssemblyItem const& _i) {
				return _i == AssemblyItem{Instruction::MSIZE} || _i.type() == VerbatimBytecode;
			});

			auto iter = m_items.begin();
			while (iter != m_items.end())
			{
				KnownState emptyState;
				CommonSubexpressionEliminator eliminator{emptyState};
				auto orig = iter;
				iter = eliminator.feedItems(iter, m_items.end(), usesMSize);
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
			if (optimisedItems.size() < m_items.size())
			{
				m_items = std::move(optimisedItems);
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

	for (auto const& i: m_items)
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

	unsigned bytesRequiredForCode = codeSize(static_cast<unsigned>(subTagSize));
	m_tagPositionsInBytecode = vector<size_t>(m_usedTags, numeric_limits<size_t>::max());
	map<size_t, pair<size_t, size_t>> tagRef;
	multimap<h256, unsigned> dataRef;
	multimap<size_t, size_t> subRef;
	vector<unsigned> sizeRef; ///< Pointers to code locations where the size of the program is inserted
	unsigned bytesPerTag = numberEncodingSize(bytesRequiredForCode);
	uint8_t tagPush = static_cast<uint8_t>(pushInstruction(bytesPerTag));

	unsigned bytesRequiredIncludingData = bytesRequiredForCode + 1 + static_cast<unsigned>(m_auxiliaryData.size());
	for (auto const& sub: m_subs)
		bytesRequiredIncludingData += static_cast<unsigned>(sub->assemble().bytecode.size());

	unsigned bytesPerDataRef = numberEncodingSize(bytesRequiredIncludingData);
	uint8_t dataRefPush = static_cast<uint8_t>(pushInstruction(bytesPerDataRef));
	ret.bytecode.reserve(bytesRequiredIncludingData);

	for (AssemblyItem const& i: m_items)
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
			unsigned b = numberEncodingSize(i.data());
			if (b == 0 && !m_evmVersion.hasPush0())
			{
				b = 1;
			}
			ret.bytecode.push_back(static_cast<uint8_t>(pushInstruction(b)));
			if (b > 0)
			{
				ret.bytecode.resize(ret.bytecode.size() + b);
				bytesRef byr(&ret.bytecode.back() + 1 - b, b);
				toBigEndian(i.data(), byr);
			}
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
			ret.bytecode.resize(ret.bytecode.size() + b);
			bytesRef byr(&ret.bytecode.back() + 1 - b, b);
			toBigEndian(s, byr);
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
			for (size_t i = 0; i < offsets.size(); ++i)
			{
				if (i != offsets.size() - 1)
				{
					ret.bytecode.push_back(uint8_t(Instruction::DUP2));
					ret.bytecode.push_back(uint8_t(Instruction::DUP2));
				}
				// TODO: should we make use of the constant optimizer methods for pushing the offsets?
				bytes offsetBytes = toCompactBigEndian(u256(offsets[i]));
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
			ret.bytecode.push_back(static_cast<uint8_t>(Instruction::JUMPDEST));
			break;
		}
		default:
			assertThrow(false, InvalidOpcode, "Unexpected opcode while assembling.");
		}
	}

	if (!immutableReferencesBySub.empty())
		throw
			langutil::Error(
				1284_error,
				langutil::Error::Type::CodeGenerationError,
				"Some immutables were read from but never assigned, possibly because of optimization."
			);

	if (!m_subs.empty() || !m_data.empty() || !m_auxiliaryData.empty())
		// Append an INVALID here to help tests find miscompilation.
		ret.bytecode.push_back(static_cast<uint8_t>(Instruction::INVALID));

	map<LinkerObject, size_t> subAssemblyOffsets;
	for (auto const& [subIdPath, bytecodeOffset]: subRef)
	{
		LinkerObject subObject = subAssemblyById(subIdPath)->assemble();
		bytesRef r(ret.bytecode.data() + bytecodeOffset, bytesPerDataRef);

		// In order for de-duplication to kick in, not only must the bytecode be identical, but
		// link and immutables references as well.
		if (size_t* subAssemblyOffset = util::valueOrNullptr(subAssemblyOffsets, subObject))
			toBigEndian(*subAssemblyOffset, r);
		else
		{
			toBigEndian(ret.bytecode.size(), r);
			subAssemblyOffsets[subObject] = ret.bytecode.size();
			ret.bytecode += subObject.bytecode;
		}
		for (auto const& ref: subObject.linkReferences)
			ret.linkReferences[ref.first + subAssemblyOffsets[subObject]] = ref.second;
	}
	for (auto const& i: tagRef)
	{
		size_t subId;
		size_t tagId;
		tie(subId, tagId) = i.second;
		assertThrow(subId == numeric_limits<size_t>::max() || subId < m_subs.size(), AssemblyException, "Invalid sub id");
		vector<size_t> const& tagPositions =
			subId == numeric_limits<size_t>::max() ?
			m_tagPositionsInBytecode :
			m_subs[subId]->m_tagPositionsInBytecode;
		assertThrow(tagId < tagPositions.size(), AssemblyException, "Reference to non-existing tag.");
		size_t pos = tagPositions[tagId];
		assertThrow(pos != numeric_limits<size_t>::max(), AssemblyException, "Reference to tag without position.");
		assertThrow(numberEncodingSize(pos) <= bytesPerTag, AssemblyException, "Tag too large for reserved space.");
		bytesRef r(ret.bytecode.data() + i.first, bytesPerTag);
		toBigEndian(pos, r);
	}
	for (auto const& [name, tagInfo]: m_namedTags)
	{
		size_t position = m_tagPositionsInBytecode.at(tagInfo.id);
		optional<size_t> tagIndex;
		for (auto&& [index, item]: m_items | ranges::views::enumerate)
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
