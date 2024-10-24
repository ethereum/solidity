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

#include <liblangutil/CharStream.h>
#include <liblangutil/Exceptions.h>

#include <libsolutil/JSON.h>
#include <libsolutil/StringUtils.h>

#include <fmt/format.h>

#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/view/drop_exactly.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/map.hpp>

#include <fstream>
#include <limits>
#include <iterator>

using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::langutil;
using namespace solidity::util;

std::map<std::string, std::shared_ptr<std::string const>> Assembly::s_sharedSourceNames;

AssemblyItem const& Assembly::append(AssemblyItem _i)
{
	assertThrow(m_deposit >= 0, AssemblyException, "Stack underflow.");
	m_deposit += static_cast<int>(_i.deposit());
	solAssert(m_currentCodeSection < m_codeSections.size());
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
		for (auto const& i: m_data)
			ret += i.second.size();

		for (auto const& codeSection: m_codeSections)
			for (AssemblyItem const& i: codeSection.items)
				ret += i.bytesRequired(tagSize, m_evmVersion, Precision::Precise);
		if (numberEncodingSize(ret) <= tagSize)
			return static_cast<unsigned>(ret);
	}
}

void Assembly::importAssemblyItemsFromJSON(Json const& _code, std::vector<std::string> const& _sourceList)
{
	// Assembly constructor creates first code section with proper type and empty `items`
	solAssert(m_codeSections.size() == 1);
	solAssert(m_codeSections[0].items.empty());
	// TODO: Add support for EOF and more than one code sections.
	solUnimplementedAssert(!m_eofVersion.has_value(), "Assembly output for EOF is not yet implemented.");
	solRequire(_code.is_array(), AssemblyImportException, "Supplied JSON is not an array.");
	for (auto jsonItemIter = std::begin(_code); jsonItemIter != std::end(_code); ++jsonItemIter)
	{
		AssemblyItem const& newItem = m_codeSections[0].items.emplace_back(createAssemblyItemFromJSON(*jsonItemIter, _sourceList));
		if (newItem == Instruction::JUMPDEST)
			solThrow(AssemblyImportException, "JUMPDEST instruction without a tag");
		else if (newItem.type() == AssemblyItemType::Tag)
		{
			++jsonItemIter;
			if (jsonItemIter != std::end(_code) && createAssemblyItemFromJSON(*jsonItemIter, _sourceList) != Instruction::JUMPDEST)
				solThrow(AssemblyImportException, "JUMPDEST expected after tag.");
		}
	}
}

AssemblyItem Assembly::createAssemblyItemFromJSON(Json const& _json, std::vector<std::string> const& _sourceList)
{
	solRequire(_json.is_object(), AssemblyImportException, "Supplied JSON is not an object.");
	static std::set<std::string> const validMembers{"name", "begin", "end", "source", "value", "modifierDepth", "jumpType"};
	for (auto const& [member, _]: _json.items())
		solRequire(
			validMembers.count(member),
			AssemblyImportException,
			fmt::format(
				"Unknown member '{}'. Valid members are: {}.",
				member,
				solidity::util::joinHumanReadable(validMembers, ", ")
			)
		);
	solRequire(isOfType<std::string>(_json["name"]), AssemblyImportException, "Member 'name' missing or not of type string.");
	solRequire(isOfTypeIfExists<int>(_json, "begin"), AssemblyImportException, "Optional member 'begin' not of type int.");
	solRequire(isOfTypeIfExists<int>(_json, "end"), AssemblyImportException, "Optional member 'end' not of type int.");
	solRequire(isOfTypeIfExists<int>(_json, "source"), AssemblyImportException, "Optional member 'source' not of type int.");
	solRequire(isOfTypeIfExists<std::string>(_json, "value"), AssemblyImportException, "Optional member 'value' not of type string.");
	solRequire(isOfTypeIfExists<int>(_json, "modifierDepth"), AssemblyImportException, "Optional member 'modifierDepth' not of type int.");
	solRequire(isOfTypeIfExists<std::string>(_json, "jumpType"), AssemblyImportException, "Optional member 'jumpType' not of type string.");

	std::string name = get<std::string>(_json["name"]);
	solRequire(!name.empty(), AssemblyImportException, "Member 'name' is empty.");

	SourceLocation location;
	if (_json.contains("begin"))
		location.start = get<int>(_json["begin"]);
	if (_json.contains("end"))
		location.end = get<int>(_json["end"]);
	int srcIndex = getOrDefault<int>(_json, "source", -1);
	size_t modifierDepth = static_cast<size_t>(getOrDefault<int>(_json, "modifierDepth", 0));
	std::string value = getOrDefault<std::string>(_json, "value", "");
	std::string jumpType = getOrDefault<std::string>(_json, "jumpType", "");

	auto updateUsedTags = [&](u256 const& data)
	{
		m_usedTags = std::max(m_usedTags, static_cast<unsigned>(data) + 1);
		return data;
	};

	auto storeImmutableHash = [&](std::string const& _immutableName) -> h256
	{
		h256 hash(util::keccak256(_immutableName));
		solAssert(m_immutables.count(hash) == 0 || m_immutables[hash] == _immutableName);
		m_immutables[hash] = _immutableName;
		return hash;
	};

	auto storeLibraryHash = [&](std::string const& _libraryName) -> h256
	{
		h256 hash(util::keccak256(_libraryName));
		solAssert(m_libraries.count(hash) == 0 || m_libraries[hash] == _libraryName);
		m_libraries[hash] = _libraryName;
		return hash;
	};

	auto requireValueDefinedForInstruction = [&](std::string const& _name, std::string const& _value)
	{
		solRequire(
			!_value.empty(),
			AssemblyImportException,
			"Member 'value' is missing for instruction '" + _name + "', but the instruction needs a value."
		);
	};

	auto requireValueUndefinedForInstruction = [&](std::string const& _name, std::string const& _value)
	{
		solRequire(
			_value.empty(),
			AssemblyImportException,
			"Member 'value' defined for instruction '" + _name + "', but the instruction does not need a value."
		);
	};

	solRequire(srcIndex >= -1 && srcIndex < static_cast<int>(_sourceList.size()), AssemblyImportException, "Source index out of bounds.");
	if (srcIndex != -1)
		location.sourceName = sharedSourceName(_sourceList[static_cast<size_t>(srcIndex)]);

	AssemblyItem result(0);

	if (c_instructions.count(name))
	{
		AssemblyItem item{c_instructions.at(name), langutil::DebugData::create(location)};
		if (!jumpType.empty())
		{
			if (item.instruction() == Instruction::JUMP || item.instruction() == Instruction::JUMPI)
			{
				std::optional<AssemblyItem::JumpType> parsedJumpType = AssemblyItem::parseJumpType(jumpType);
				if (!parsedJumpType.has_value())
					solThrow(AssemblyImportException, "Invalid jump type.");
				item.setJumpType(parsedJumpType.value());
			}
			else
				solThrow(
					AssemblyImportException,
					"Member 'jumpType' set on instruction different from JUMP or JUMPI (was set on instruction '" + name + "')"
				);
		}
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
			requireValueDefinedForInstruction(name, value);
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

std::string locationFromSources(StringMap const& _sourceCodes, SourceLocation const& _location)
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
	Functionalizer (std::ostream& _out, std::string const& _prefix, StringMap const& _sourceCodes, Assembly const& _assembly):
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

		std::string expression = _item.toAssemblyText(m_assembly);

		if (!(
			_item.canBeFunctional() &&
			_item.returnValues() <= 1 &&
			_item.arguments() <= m_pending.size()
		))
		{
			flush();
			m_out << m_prefix << (_item.type() == Tag ? "" : "  ") << expression << std::endl;
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
		for (std::string const& expression: m_pending)
			m_out << m_prefix << "  " << expression << std::endl;
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
				m_out << ":" << std::to_string(m_location.start) + ":" + std::to_string(m_location.end);
		}

		if (_debugInfoSelection.snippet)
		{
			if (_debugInfoSelection.location)
				m_out << "  ";

			m_out << locationFromSources(m_sourceCodes, m_location);
		}

		m_out << " */" << std::endl;
	}

private:
	strings m_pending;
	SourceLocation m_location;

	std::ostream& m_out;
	std::string const& m_prefix;
	StringMap const& m_sourceCodes;
	Assembly const& m_assembly;
};

}

void Assembly::assemblyStream(
	std::ostream& _out,
	DebugInfoSelection const& _debugInfoSelection,
	std::string const& _prefix,
	StringMap const& _sourceCodes
) const
{
	Functionalizer f(_out, _prefix, _sourceCodes, *this);

	for (auto const& i: m_codeSections.front().items)
		f.feed(i, _debugInfoSelection);
	f.flush();

	// Implementing this requires introduction of CALLF, RETF and JUMPF
	if (m_codeSections.size() > 1)
		solUnimplemented("Add support for more code sections");

	if (!m_data.empty() || !m_subs.empty())
	{
		_out << _prefix << "stop" << std::endl;
		for (auto const& i: m_data)
			if (u256(i.first) >= m_subs.size())
				_out << _prefix << "data_" << toHex(u256(i.first)) << " " << util::toHex(i.second) << std::endl;

		for (size_t i = 0; i < m_subs.size(); ++i)
		{
			_out << std::endl << _prefix << "sub_" << i << ": assembly {\n";
			m_subs[i]->assemblyStream(_out, _debugInfoSelection, _prefix + "    ", _sourceCodes);
			_out << _prefix << "}" << std::endl;
		}
	}

	if (m_auxiliaryData.size() > 0)
		_out << std::endl << _prefix << "auxdata: 0x" << util::toHex(m_auxiliaryData) << std::endl;
}

std::string Assembly::assemblyString(
	DebugInfoSelection const& _debugInfoSelection,
	StringMap const& _sourceCodes
) const
{
	std::ostringstream tmp;
	assemblyStream(tmp, _debugInfoSelection, "", _sourceCodes);
	return tmp.str();
}

Json Assembly::assemblyJSON(std::map<std::string, unsigned> const& _sourceIndices, bool _includeSourceList) const
{
	Json root;
	root[".code"] = Json::array();
	Json& code = root[".code"];
    // TODO: support EOF
	solUnimplementedAssert(!m_eofVersion.has_value(), "Assembly output for EOF is not yet implemented.");
	solAssert(m_codeSections.size() == 1);
	for (AssemblyItem const& item: m_codeSections.front().items)
	{
		int sourceIndex = -1;
		if (item.location().sourceName)
		{
			auto iter = _sourceIndices.find(*item.location().sourceName);
			if (iter != _sourceIndices.end())
				sourceIndex = static_cast<int>(iter->second);
		}

		auto [name, data] = item.nameAndData(m_evmVersion);
		Json jsonItem;
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
		code.emplace_back(std::move(jsonItem));

		if (item.type() == AssemblyItemType::Tag)
		{
			Json jumpdest;
			jumpdest["name"] = "JUMPDEST";
			jumpdest["begin"] = item.location().start;
			jumpdest["end"] = item.location().end;
			jumpdest["source"] = sourceIndex;
			if (item.m_modifierDepth != 0)
				jumpdest["modifierDepth"] = static_cast<int>(item.m_modifierDepth);
			code.emplace_back(std::move(jumpdest));
		}
	}
	if (_includeSourceList)
	{
		root["sourceList"] = Json::array();
		Json& jsonSourceList = root["sourceList"];
		unsigned maxSourceIndex = 0;
		for (auto const& [sourceName, sourceIndex]: _sourceIndices)
		{
			maxSourceIndex = std::max(sourceIndex, maxSourceIndex);
			jsonSourceList[sourceIndex] = sourceName;
		}
		solAssert(maxSourceIndex + 1 >= _sourceIndices.size());
		solRequire(
			_sourceIndices.size() == 0 || _sourceIndices.size() == maxSourceIndex + 1,
			AssemblyImportException,
			"The 'sourceList' array contains invalid 'null' item."
		);
	}

	if (!m_data.empty() || !m_subs.empty())
	{
		root[".data"] = Json::object();
		Json& data = root[".data"];
		for (auto const& i: m_data)
			if (u256(i.first) >= m_subs.size())
				data[util::toHex(toBigEndian((u256)i.first), util::HexPrefix::DontAdd, util::HexCase::Upper)] = util::toHex(i.second);

		for (size_t i = 0; i < m_subs.size(); ++i)
		{
			std::stringstream hexStr;
			hexStr << std::hex << i;
			data[hexStr.str()] = m_subs[i]->assemblyJSON(_sourceIndices, /*_includeSourceList = */false);
		}
	}

	if (!m_auxiliaryData.empty())
		root[".auxdata"] = util::toHex(m_auxiliaryData);

	return root;
}

std::pair<std::shared_ptr<Assembly>, std::vector<std::string>> Assembly::fromJSON(
	Json const& _json,
	std::vector<std::string> const& _sourceList,
	size_t _level,
	std::optional<uint8_t> _eofVersion
)
{
	solRequire(_json.is_object(), AssemblyImportException, "Supplied JSON is not an object.");
	static std::set<std::string> const validMembers{".code", ".data", ".auxdata", "sourceList"};
	for (auto const& [attribute, _]: _json.items())
		solRequire(validMembers.count(attribute), AssemblyImportException, "Unknown attribute '" + attribute + "'.");

	if (_level == 0)
	{
		if (_json.contains("sourceList"))
		{
			solRequire(_json["sourceList"].is_array(), AssemblyImportException, "Optional member 'sourceList' is not an array.");
			for (Json const& sourceName: _json["sourceList"])
			{
				solRequire(!sourceName.is_null(), AssemblyImportException, "The 'sourceList' array contains invalid 'null' item.");
				solRequire(
					sourceName.is_string(),
					AssemblyImportException,
					"The 'sourceList' array contains an item that is not a string."
				);
			}
		}
	}
	else
		solRequire(
			!_json.contains("sourceList"),
			AssemblyImportException,
			"Member 'sourceList' may only be present in the root JSON object."
		);

	auto result = std::make_shared<Assembly>(EVMVersion{}, _level == 0 /* _creation */, _eofVersion, "" /* _name */);
	std::vector<std::string> parsedSourceList;
	if (_json.contains("sourceList"))
	{
		solAssert(_level == 0);
		solAssert(_sourceList.empty());
		for (Json const& sourceName: _json["sourceList"])
		{
			solRequire(
				std::find(parsedSourceList.begin(), parsedSourceList.end(), sourceName.get<std::string>()) == parsedSourceList.end(),
				AssemblyImportException,
				"Items in 'sourceList' array are not unique."
			);
			parsedSourceList.emplace_back(sourceName.get<std::string>());
		}
	}

	solRequire(_json.contains(".code"), AssemblyImportException, "Member '.code' is missing.");
	solRequire(_json[".code"].is_array(), AssemblyImportException, "Member '.code' is not an array.");
	for (Json const& codeItem: _json[".code"])
		solRequire(codeItem.is_object(), AssemblyImportException, "The '.code' array contains an item that is not an object.");

	result->importAssemblyItemsFromJSON(_json[".code"], _level == 0 ? parsedSourceList : _sourceList);

	if (_json.contains(".auxdata"))
	{
		solRequire(_json[".auxdata"].is_string(), AssemblyImportException, "Optional member '.auxdata' is not a string.");
		result->m_auxiliaryData = fromHex(_json[".auxdata"].get<std::string>());
		solRequire(!result->m_auxiliaryData.empty(), AssemblyImportException, "Optional member '.auxdata' is not a valid hexadecimal string.");
	}

	if (_json.contains(".data"))
	{
		solRequire(_json[".data"].is_object(), AssemblyImportException, "Optional member '.data' is not an object.");
		Json const& data = _json[".data"];
		std::map<size_t, std::shared_ptr<Assembly>> subAssemblies;
		for (auto const& [key, value] : data.items())
		{
			if (value.is_string())
			{
				solRequire(
					value.get<std::string>().empty() || !fromHex(value.get<std::string>()).empty(),
					AssemblyImportException,
					"The value for key '" + key + "' inside '.data' is not a valid hexadecimal string."
				);
				result->m_data[h256(fromHex(key))] = fromHex(value.get<std::string>());
			}
			else if (value.is_object())
			{
				size_t index{};
				try
				{
					// Using signed variant because stoul() still accepts negative numbers and
					// just lets them wrap around.
					int parsedDataItemID = std::stoi(key, nullptr, 16);
					solRequire(parsedDataItemID >= 0, AssemblyImportException, "The key '" + key + "' inside '.data' is out of the supported integer range.");
					index = static_cast<size_t>(parsedDataItemID);
				}
				catch (std::invalid_argument const&)
				{
					solThrow(AssemblyImportException, "The key '" + key + "' inside '.data' is not an integer.");
				}
				catch (std::out_of_range const&)
				{
					solThrow(AssemblyImportException, "The key '" + key + "' inside '.data' is out of the supported integer range.");
				}

				auto [subAssembly, emptySourceList] = Assembly::fromJSON(value, _level == 0 ? parsedSourceList : _sourceList, _level + 1, _eofVersion);
				solAssert(subAssembly);
				solAssert(emptySourceList.empty());
				solAssert(subAssemblies.count(index) == 0);
				subAssemblies[index] = subAssembly;
			}
			else
				solThrow(AssemblyImportException, "The value of key '" + key + "' inside '.data' is neither a hex string nor an object.");
		}

		if (!subAssemblies.empty())
			solRequire(
				ranges::max(subAssemblies | ranges::views::keys) == subAssemblies.size() - 1,
				AssemblyImportException,
				fmt::format(
					"Invalid subassembly indices in '.data'. Not all numbers between 0 and {} are present.",
					subAssemblies.size() - 1
				)
			);

		result->m_subs = subAssemblies | ranges::views::values | ranges::to<std::vector>;
	}

	if (_level == 0)
		result->encodeAllPossibleSubPathsInAssemblyTree();

	return std::make_pair(result, _level == 0 ? parsedSourceList : std::vector<std::string>{});
}

void Assembly::encodeAllPossibleSubPathsInAssemblyTree(std::vector<size_t> _pathFromRoot, std::vector<Assembly*> _assembliesOnPath)
{
	_assembliesOnPath.push_back(this);
	for (_pathFromRoot.push_back(0); _pathFromRoot.back() < m_subs.size(); ++_pathFromRoot.back())
	{
		for (size_t distanceFromRoot = 0; distanceFromRoot < _assembliesOnPath.size(); ++distanceFromRoot)
			_assembliesOnPath[distanceFromRoot]->encodeSubPath(
				_pathFromRoot | ranges::views::drop_exactly(distanceFromRoot) | ranges::to<std::vector>
			);

		m_subs[_pathFromRoot.back()]->encodeAllPossibleSubPathsInAssemblyTree(_pathFromRoot, _assembliesOnPath);
	}
}

std::shared_ptr<std::string const> Assembly::sharedSourceName(std::string const& _name) const
{
	if (s_sharedSourceNames.find(_name) == s_sharedSourceNames.end())
		s_sharedSourceNames[_name] = std::make_shared<std::string>(_name);

	return s_sharedSourceNames[_name];
}

AssemblyItem Assembly::namedTag(std::string const& _name, size_t _params, size_t _returns, std::optional<uint64_t> _sourceID)
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

AssemblyItem Assembly::newPushLibraryAddress(std::string const& _identifier)
{
	h256 h(util::keccak256(_identifier));
	m_libraries[h] = _identifier;
	return AssemblyItem{PushLibraryAddress, h};
}

AssemblyItem Assembly::newPushImmutable(std::string const& _identifier)
{
	h256 h(util::keccak256(_identifier));
	m_immutables[h] = _identifier;
	return AssemblyItem{PushImmutable, h};
}

AssemblyItem Assembly::newImmutableAssignment(std::string const& _identifier)
{
	h256 h(util::keccak256(_identifier));
	m_immutables[h] = _identifier;
	return AssemblyItem{AssignImmutable, h};
}

AssemblyItem Assembly::newAuxDataLoadN(size_t _offset)
{
	return AssemblyItem{AuxDataLoadN, _offset};
}

Assembly& Assembly::optimise(OptimiserSettings const& _settings)
{
	optimiseInternal(_settings, {});
	return *this;
}

std::map<u256, u256> const& Assembly::optimiseInternal(
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
		std::map<u256, u256> const& subTagReplacements = sub.optimiseInternal(
			settings,
			referencedTags
		);
		// Apply the replacements (can be empty).
		for (auto& codeSection: m_codeSections)
			BlockDeduplicator::applyTagReplacement(codeSection.items, subTagReplacements, subId);
	}

	std::map<u256, u256> tagReplacements;
	// Iterate until no new optimisation possibilities are found.
	for (unsigned count = 1; count > 0;)
	{
		count = 0;

		// TODO: verify this for EOF.
		if (_settings.runInliner && !m_eofVersion.has_value())
		{
			solAssert(m_codeSections.size() == 1);
			Inliner{
				m_codeSections.front().items,
				_tagsReferencedFromOutside,
				_settings.expectedExecutionsPerDeployment,
				isCreation(),
				_settings.evmVersion}
				.optimise();
		}
		// TODO: verify this for EOF.
		if (_settings.runJumpdestRemover && !m_eofVersion.has_value())
		{
			for (auto& codeSection: m_codeSections)
			{
				JumpdestRemover jumpdestOpt{codeSection.items};
				if (jumpdestOpt.optimise(_tagsReferencedFromOutside))
					count++;
			}
		}

		// TODO: verify this for EOF.
		if (_settings.runPeephole && !m_eofVersion.has_value())
		{
			for (auto& codeSection: m_codeSections)
			{
				PeepholeOptimiser peepOpt{codeSection.items, m_evmVersion};
				while (peepOpt.optimise())
				{
					count++;
					assertThrow(count < 64000, OptimizerException, "Peephole optimizer seems to be stuck.");
				}
			}
		}

		// This only modifies PushTags, we have to run again to actually remove code.
		// TODO: implement for EOF.
		if (_settings.runDeduplicate && !m_eofVersion.has_value())
			for (auto& section: m_codeSections)
			{
				BlockDeduplicator deduplicator{section.items};
				if (deduplicator.deduplicate())
				{
					for (auto const& replacement: deduplicator.replacedTags())
					{
						assertThrow(
							replacement.first <= std::numeric_limits<size_t>::max() && replacement.second <= std::numeric_limits<size_t>::max(),
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

			solAssert(m_codeSections.size() == 1);
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

	// TODO: investigate for EOF
	if (_settings.runConstantOptimiser && !m_eofVersion.has_value())
		ConstantOptimisationMethod::optimiseConstants(
			isCreation(),
			isCreation() ? 1 : _settings.expectedExecutionsPerDeployment,
			_settings.evmVersion,
			*this
		);

	m_tagReplacements = std::move(tagReplacements);
	return *m_tagReplacements;
}

namespace
{
template<typename ValueT>
void setBigEndian(bytes& _dest, size_t _offset, size_t _size, ValueT _value)
{
	assertThrow(numberEncodingSize(_value) <= _size, AssemblyException, "");
	toBigEndian(_value, bytesRef(_dest.data() + _offset, _size));
}

template<typename ValueT>
void appendBigEndian(bytes& _dest, size_t _size, ValueT _value)
{
	_dest.resize(_dest.size() + _size);
	setBigEndian(_dest, _dest.size() - _size, _size, _value);
}

template<typename ValueT>
void setBigEndianUint16(bytes& _dest, size_t _offset, ValueT _value)
{
	setBigEndian(_dest, _offset, 2, _value);
}

template<typename ValueT>
void appendBigEndianUint16(bytes& _dest, ValueT _value)
{
	static_assert(!std::numeric_limits<ValueT>::is_signed, "only unsigned types or bigint supported");
	assertThrow(_value <= 0xFFFF, AssemblyException, "");
	appendBigEndian(_dest, 2, static_cast<size_t>(_value));
}
}

std::tuple<bytes, std::vector<size_t>, size_t> Assembly::createEOFHeader(std::set<uint16_t> const& _referencedSubIds) const
{
	bytes retBytecode;
	std::vector<size_t> codeSectionSizePositions;
	size_t dataSectionSizePosition;

	retBytecode.push_back(0xef);
	retBytecode.push_back(0x00);
	retBytecode.push_back(0x01);                                        // version 1

	retBytecode.push_back(0x01);                                        // kind=type
	appendBigEndianUint16(retBytecode, m_codeSections.size() * 4u);     // length of type section

	retBytecode.push_back(0x02);                                        // kind=code
	appendBigEndianUint16(retBytecode, m_codeSections.size());          // placeholder for number of code sections

	for (auto const& codeSection: m_codeSections)
	{
		(void) codeSection;
		codeSectionSizePositions.emplace_back(retBytecode.size());
		appendBigEndianUint16(retBytecode, 0u);                         // placeholder for length of code
	}

	if (!_referencedSubIds.empty())
	{
		retBytecode.push_back(0x03);
		appendBigEndianUint16(retBytecode, _referencedSubIds.size());

		for (auto subId: _referencedSubIds)
			appendBigEndianUint16(retBytecode, m_subs[subId]->assemble().bytecode.size());
	}

	retBytecode.push_back(0x04);                                        // kind=data
	dataSectionSizePosition = retBytecode.size();
	appendBigEndianUint16(retBytecode, 0u);                             // length of data

	retBytecode.push_back(0x00);                                        // terminator

	for (auto const& codeSection: m_codeSections)
	{
		retBytecode.push_back(codeSection.inputs);
		retBytecode.push_back(codeSection.outputs);
		// TODO: Add stack height calculation
		appendBigEndianUint16(retBytecode, 0xFFFFu);
	}

	return {retBytecode, codeSectionSizePositions, dataSectionSizePosition};
}

LinkerObject const& Assembly::assemble() const
{
	solRequire(!m_invalid, AssemblyException, "Attempted to assemble invalid Assembly object.");
	// Return the already assembled object, if present.
	if (!m_assembledObject.bytecode.empty())
		return m_assembledObject;

	// Otherwise ensure the object is actually clear.
	solRequire(m_assembledObject.linkReferences.empty(), AssemblyException, "Unexpected link references.");

	bool const eof = m_eofVersion.has_value();
	solRequire(!eof || m_eofVersion == 1, AssemblyException, "Invalid EOF version.");

	if (!eof)
		return assembleLegacy();
	else
		return assembleEOF();
}

[[nodiscard]] bytes Assembly::assembleOperation(AssemblyItem const& _item) const
{
	// solidity::evmasm::Instructions underlying type is uint8_t
	// TODO: Change to std::to_underlying since C++23
	return {static_cast<uint8_t>(_item.instruction())};
}

[[nodiscard]] bytes Assembly::assemblePush(AssemblyItem const& _item) const
{
	bytes ret;
	unsigned pushValueSize = numberEncodingSize(_item.data());
	if (pushValueSize == 0 && !m_evmVersion.hasPush0())
		pushValueSize = 1;

	// solidity::evmasm::Instructions underlying type is uint8_t
	// TODO: Change to std::to_underlying since C++23
	ret.push_back(static_cast<uint8_t>(pushInstruction(pushValueSize)));
	if (pushValueSize > 0)
		appendBigEndian(ret, pushValueSize, _item.data());

	return ret;
}

[[nodiscard]] std::pair<bytes, Assembly::LinkRef> Assembly::assemblePushLibraryAddress(AssemblyItem const& _item, size_t _pos) const
{
	return {
		// solidity::evmasm::Instructions underlying type is uint8_t
		// TODO: Change to std::to_underlying since C++23
		bytes(1, static_cast<uint8_t>(Instruction::PUSH20)) + bytes(20),
		{_pos + 1, m_libraries.at(_item.data())}
	};
}

[[nodiscard]] bytes Assembly::assembleVerbatimBytecode(AssemblyItem const& item) const
{
	return item.verbatimData();
}

[[nodiscard]] bytes Assembly::assemblePushDeployTimeAddress() const
{
	// solidity::evmasm::Instructions underlying type is uint8_t
	// TODO: Change to std::to_underlying since C++23
	return bytes(1, static_cast<uint8_t>(Instruction::PUSH20)) + bytes(20);
}

[[nodiscard]] bytes Assembly::assembleTag(AssemblyItem const& _item, size_t _pos, bool _addJumpDest) const
{
	solRequire(_item.data() != 0, AssemblyException, "Invalid tag position.");
	solRequire(_item.splitForeignPushTag().first == std::numeric_limits<size_t>::max(), AssemblyException, "Foreign tag.");
	solRequire(_pos < 0xffffffffL, AssemblyException, "Tag too large.");
	size_t tagId = static_cast<size_t>(_item.data());
	solRequire(m_tagPositionsInBytecode[tagId] == std::numeric_limits<size_t>::max(), AssemblyException, "Duplicate tag position.");
	m_tagPositionsInBytecode[tagId] = _pos;

	// solidity::evmasm::Instructions underlying type is uint8_t
	// TODO: Change to std::to_underlying since C++23
	return _addJumpDest ? bytes(1, static_cast<uint8_t>(Instruction::JUMPDEST)) : bytes();
}

LinkerObject const& Assembly::assembleLegacy() const
{
	solAssert(!m_eofVersion.has_value());
	solAssert(!m_invalid);
	// Return the already assembled object, if present.
	if (!m_assembledObject.bytecode.empty())
		return m_assembledObject;
	// Otherwise ensure the object is actually clear.
	solAssert(m_assembledObject.linkReferences.empty());

	LinkerObject& ret = m_assembledObject;

	size_t subTagSize = 1;
	std::map<u256, LinkerObject::ImmutableRefs> immutableReferencesBySub;
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
			if (tagPos != std::numeric_limits<size_t>::max() && numberEncodingSize(tagPos) > subTagSize)
				subTagSize = numberEncodingSize(tagPos);
	}

	bool setsImmutables = false;
	bool pushesImmutables = false;

	assertThrow(m_codeSections.size() == 1, AssemblyException, "Expected exactly one code section in non-EOF code.");
	AssemblyItems const& items = m_codeSections.front().items;

	for (auto const& item: items)
		if (item.type() == AssignImmutable)
		{
			item.setImmutableOccurrences(immutableReferencesBySub[item.data()].second.size());
			setsImmutables = true;
		}
		else if (item.type() == PushImmutable)
			pushesImmutables = true;
	if (setsImmutables || pushesImmutables)
		assertThrow(
			setsImmutables != pushesImmutables,
			AssemblyException,
			"Cannot push and assign immutables in the same assembly subroutine."
		);

	unsigned bytesRequiredForCode = codeSize(static_cast<unsigned>(subTagSize));
	m_tagPositionsInBytecode = std::vector<size_t>(m_usedTags, std::numeric_limits<size_t>::max());
	unsigned bytesPerTag = numberEncodingSize(bytesRequiredForCode);
	// Adjust bytesPerTag for references to sub assemblies.
	for (AssemblyItem const& item: items)
		if (item.type() == PushTag)
		{
			auto [subId, tagId] = item.splitForeignPushTag();
			if (subId == std::numeric_limits<size_t>::max())
				continue;
			assertThrow(subId < m_subs.size(), AssemblyException, "Invalid sub id");
			auto subTagPosition = m_subs[subId]->m_tagPositionsInBytecode.at(tagId);
			assertThrow(subTagPosition != std::numeric_limits<size_t>::max(), AssemblyException, "Reference to tag without position.");
			bytesPerTag = std::max(bytesPerTag, numberEncodingSize(subTagPosition));
		}

	unsigned bytesRequiredIncludingData = bytesRequiredForCode + 1 + static_cast<unsigned>(m_auxiliaryData.size());
	for (auto const& sub: m_subs)
		bytesRequiredIncludingData += static_cast<unsigned>(sub->assemble().bytecode.size());

	unsigned bytesPerDataRef = numberEncodingSize(bytesRequiredIncludingData);
	ret.bytecode.reserve(bytesRequiredIncludingData);

	TagRefs tagRefs;
	DataRefs dataRefs;
	SubAssemblyRefs subRefs;
	ProgramSizeRefs sizeRefs;
	uint8_t tagPush = static_cast<uint8_t>(pushInstruction(bytesPerTag));
	uint8_t dataRefPush = static_cast<uint8_t>(pushInstruction(bytesPerDataRef));

	for (AssemblyItem const& item: items)
	{
		// store position of the invalid jump destination
		if (item.type() != Tag && m_tagPositionsInBytecode[0] == std::numeric_limits<size_t>::max())
			m_tagPositionsInBytecode[0] = ret.bytecode.size();

		switch (item.type())
		{
		case Operation:
			ret.bytecode += assembleOperation(item);
			break;
		case Push:
			ret.bytecode += assemblePush(item);
			break;
		case PushTag:
		{
			ret.bytecode.push_back(tagPush);
			tagRefs[ret.bytecode.size()] = item.splitForeignPushTag();
			ret.bytecode.resize(ret.bytecode.size() + bytesPerTag);
			break;
		}
		case PushData:
			ret.bytecode.push_back(dataRefPush);
			dataRefs.insert(std::make_pair(h256(item.data()), ret.bytecode.size()));
			ret.bytecode.resize(ret.bytecode.size() + bytesPerDataRef);
			break;
		case PushSub:
			assertThrow(item.data() <= std::numeric_limits<size_t>::max(), AssemblyException, "");
			ret.bytecode.push_back(dataRefPush);
			subRefs.insert(std::make_pair(static_cast<size_t>(item.data()), ret.bytecode.size()));
			ret.bytecode.resize(ret.bytecode.size() + bytesPerDataRef);
			break;
		case PushSubSize:
		{
			assertThrow(item.data() <= std::numeric_limits<size_t>::max(), AssemblyException, "");
			auto s = subAssemblyById(static_cast<size_t>(item.data()))->assemble().bytecode.size();
			item.setPushedValue(u256(s));
			unsigned b = std::max<unsigned>(1, numberEncodingSize(s));
			ret.bytecode.push_back(static_cast<uint8_t>(pushInstruction(b)));
			ret.bytecode.resize(ret.bytecode.size() + b);
			bytesRef byr(&ret.bytecode.back() + 1 - b, b);
			toBigEndian(s, byr);
			break;
		}
		case PushProgramSize:
		{
			ret.bytecode.push_back(dataRefPush);
			sizeRefs.push_back(static_cast<unsigned>(ret.bytecode.size()));
			ret.bytecode.resize(ret.bytecode.size() + bytesPerDataRef);
			break;
		}
		case PushLibraryAddress:
		{
			auto const [bytecode, linkRef] = assemblePushLibraryAddress(item, ret.bytecode.size());
			ret.bytecode += bytecode;
			ret.linkReferences.insert(linkRef);
			break;
		}
		case PushImmutable:
			ret.bytecode.push_back(static_cast<uint8_t>(Instruction::PUSH32));
			// Maps keccak back to the "identifier" std::string of that immutable.
			ret.immutableReferences[item.data()].first = m_immutables.at(item.data());
			// Record the bytecode offset of the PUSH32 argument.
			ret.immutableReferences[item.data()].second.emplace_back(ret.bytecode.size());
			// Advance bytecode by 32 bytes (default initialized).
			ret.bytecode.resize(ret.bytecode.size() + 32);
			break;
		case VerbatimBytecode:
			ret.bytecode += assembleVerbatimBytecode(item);
			break;
		case AssignImmutable:
		{
			// Expect 2 elements on stack (source, dest_base)
			auto const& offsets = immutableReferencesBySub[item.data()].second;
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
			immutableReferencesBySub.erase(item.data());
			break;
		}
		case PushDeployTimeAddress:
			ret.bytecode += assemblePushDeployTimeAddress();
			break;
		case Tag:
			ret.bytecode += assembleTag(item, ret.bytecode.size(), true);
			break;
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

	std::map<LinkerObject, size_t> subAssemblyOffsets;
	for (auto const& [subIdPath, bytecodeOffset]: subRefs)
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
	for (auto const& i: tagRefs)
	{
		size_t subId;
		size_t tagId;
		std::tie(subId, tagId) = i.second;
		assertThrow(subId == std::numeric_limits<size_t>::max() || subId < m_subs.size(), AssemblyException, "Invalid sub id");
		std::vector<size_t> const& tagPositions =
			subId == std::numeric_limits<size_t>::max() ?
			m_tagPositionsInBytecode :
			m_subs[subId]->m_tagPositionsInBytecode;
		assertThrow(tagId < tagPositions.size(), AssemblyException, "Reference to non-existing tag.");
		size_t pos = tagPositions[tagId];
		assertThrow(pos != std::numeric_limits<size_t>::max(), AssemblyException, "Reference to tag without position.");
		assertThrow(numberEncodingSize(pos) <= bytesPerTag, AssemblyException, "Tag too large for reserved space.");
		bytesRef r(ret.bytecode.data() + i.first, bytesPerTag);
		toBigEndian(pos, r);
	}
	for (auto const& [name, tagInfo]: m_namedTags)
	{
		size_t position = m_tagPositionsInBytecode.at(tagInfo.id);
		std::optional<size_t> tagIndex;
		for (auto&& [index, item]: items | ranges::views::enumerate)
			if (item.type() == Tag && static_cast<size_t>(item.data()) == tagInfo.id)
			{
				tagIndex = index;
				break;
			}
		ret.functionDebugData[name] = {
			position == std::numeric_limits<size_t>::max() ? std::nullopt : std::optional<size_t>{position},
			tagIndex,
			tagInfo.sourceID,
			tagInfo.params,
			tagInfo.returns
		};
	}

	for (auto const& dataItem: m_data)
	{
		auto references = dataRefs.equal_range(dataItem.first);
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

	for (unsigned pos: sizeRefs)
	{
		bytesRef r(ret.bytecode.data() + pos, bytesPerDataRef);
		toBigEndian(ret.bytecode.size(), r);
	}
	return ret;
}

std::map<uint16_t, uint16_t> Assembly::findReferencedContainers() const
{
	std::set<uint16_t> referencedSubcontainersIds;
	solAssert(m_subs.size() <= 0x100); // According to EOF spec

	for (auto&& codeSection: m_codeSections)
		for (AssemblyItem const& item: codeSection.items)
			if (item.type() == EOFCreate || item.type() == ReturnContract)
			{
				solAssert(item.data() <= m_subs.size(), "Invalid subcontainer index.");
				auto const containerId = static_cast<ContainerID>(item.data());
				referencedSubcontainersIds.insert(containerId);
			}

	std::map<uint16_t, uint16_t> replacements;
	uint8_t nUnreferenced = 0;
	for (uint8_t i = 0; i < static_cast<uint16_t>(m_subs.size()); ++i)
	{
		if (referencedSubcontainersIds.count(i) > 0)
			replacements[i] = static_cast<uint16_t>(i - nUnreferenced);
		else
			nUnreferenced++;
	}

	return replacements;
}

std::optional<uint16_t> Assembly::findMaxAuxDataLoadNOffset() const
{
	std::optional<unsigned> maxOffset = std::nullopt;
	for (auto&& codeSection: m_codeSections)
		for (AssemblyItem const& item: codeSection.items)
			if (item.type() == AuxDataLoadN)
			{
				solAssert(item.data() <= std::numeric_limits<uint16_t>::max(), "Invalid auxdataloadn index value.");
				auto const offset = static_cast<unsigned>(item.data());
				if (!maxOffset.has_value() || offset > maxOffset.value())
					maxOffset = offset;

			}

	return maxOffset;
}

LinkerObject const& Assembly::assembleEOF() const
{
	solAssert(m_eofVersion.has_value() && m_eofVersion == 1);
	LinkerObject& ret = m_assembledObject;

	auto const subIdsReplacements = findReferencedContainers();
	auto const referencedSubIds = keys(subIdsReplacements);

	solRequire(!m_codeSections.empty(), AssemblyException, "Expected at least one code section.");
	solRequire(
		m_codeSections.front().inputs == 0 && m_codeSections.front().outputs == 0x80, AssemblyException,
		"Expected the first code section to have zero inputs and be non-returning."
	);

	auto const maxAuxDataLoadNOffset = findMaxAuxDataLoadNOffset();

	// Insert EOF1 header.
	auto [headerBytecode, codeSectionSizePositions, dataSectionSizePosition] = createEOFHeader(referencedSubIds);
	ret.bytecode = headerBytecode;

	m_tagPositionsInBytecode = std::vector<size_t>(m_usedTags, std::numeric_limits<size_t>::max());
	std::map<size_t, uint16_t> dataSectionRef;

	for (auto&& [codeSectionIndex, codeSection]: m_codeSections | ranges::views::enumerate)
	{
		auto const sectionStart = ret.bytecode.size();
		for (AssemblyItem const& item: codeSection.items)
		{
			// store position of the invalid jump destination
			if (item.type() != Tag && m_tagPositionsInBytecode[0] == std::numeric_limits<size_t>::max())
				m_tagPositionsInBytecode[0] = ret.bytecode.size();

			switch (item.type())
			{
			case Operation:
				solAssert(
					item.instruction() != Instruction::DATALOADN &&
					item.instruction() != Instruction::RETURNCONTRACT &&
					item.instruction() != Instruction::EOFCREATE
				);
				solAssert(!(item.instruction() >= Instruction::PUSH0 && item.instruction() <= Instruction::PUSH32));
				ret.bytecode += assembleOperation(item);
				break;
			case Push:
				ret.bytecode += assemblePush(item);
				break;
			case PushLibraryAddress:
			{
				auto const [pushLibraryAddressBytecode, linkRef] = assemblePushLibraryAddress(item, ret.bytecode.size());
				ret.bytecode += pushLibraryAddressBytecode;
				ret.linkReferences.insert(linkRef);
				break;
			}
			case EOFCreate:
			{
				ret.bytecode.push_back(static_cast<uint8_t>(Instruction::EOFCREATE));
				ret.bytecode.push_back(static_cast<uint8_t>(item.data()));
				break;
			}
			case ReturnContract:
			{
				ret.bytecode.push_back(static_cast<uint8_t>(Instruction::RETURNCONTRACT));
				ret.bytecode.push_back(static_cast<uint8_t>(item.data()));
				break;
			}
			case VerbatimBytecode:
				ret.bytecode += assembleVerbatimBytecode(item);
				break;
			case PushDeployTimeAddress:
				ret.bytecode += assemblePushDeployTimeAddress();
				break;
			case Tag:
				ret.bytecode += assembleTag(item, ret.bytecode.size(), false);
				break;
			case AuxDataLoadN:
			{
				// In findMaxAuxDataLoadNOffset we already verified that unsigned data value fits 2 bytes
				solAssert(item.data() <= std::numeric_limits<uint16_t>::max(), "Invalid auxdataloadn position.");
				ret.bytecode.push_back(uint8_t(Instruction::DATALOADN));
				dataSectionRef[ret.bytecode.size()] = static_cast<uint16_t>(item.data());
				appendBigEndianUint16(ret.bytecode, item.data());
				break;
			}
			default:
				solThrow(InvalidOpcode, "Unexpected opcode while assembling.");
			}
		}

		setBigEndianUint16(ret.bytecode, codeSectionSizePositions[codeSectionIndex], ret.bytecode.size() - sectionStart);
	}

	for (auto i: referencedSubIds)
		ret.bytecode += m_subs[i]->assemble().bytecode;

	// TODO: Fill functionDebugData for EOF. It probably should be handled for new code section in the loop above.
	solRequire(m_namedTags.empty(), AssemblyException, "Named tags must be empty in EOF context.");

	auto const dataStart = ret.bytecode.size();

	for (auto const& dataItem: m_data)
		ret.bytecode += dataItem.second;

	ret.bytecode += m_auxiliaryData;

	auto const preDeployDataSectionSize = ret.bytecode.size() - dataStart;
	// DATALOADN loads 32 bytes from EOF data section zero padded if reading out of data bounds.
	// In our case we do not allow DATALOADN with offsets which reads out of data bounds.
	auto const staticAuxDataSize = maxAuxDataLoadNOffset.has_value() ? (*maxAuxDataLoadNOffset + 32u) : 0u;
	solRequire(preDeployDataSectionSize + staticAuxDataSize < std::numeric_limits<uint16_t>::max(), AssemblyException,
		"Invalid DATALOADN offset.");

	// If some data was already added to data section we need to update data section refs accordingly
	if (preDeployDataSectionSize > 0)
		for (auto [refPosition, staticAuxDataOffset] : dataSectionRef)
		{
			// staticAuxDataOffset + preDeployDataSectionSize value is already verified to fit 2 bytes because
			// staticAuxDataOffset < staticAuxDataSize
			setBigEndianUint16(ret.bytecode, refPosition, staticAuxDataOffset + preDeployDataSectionSize);
		}

	auto const preDeployAndStaticAuxDataSize = preDeployDataSectionSize + staticAuxDataSize;

	setBigEndianUint16(ret.bytecode, dataSectionSizePosition, preDeployAndStaticAuxDataSize);

	return ret;
}

std::vector<size_t> Assembly::decodeSubPath(size_t _subObjectId) const
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

size_t Assembly::encodeSubPath(std::vector<size_t> const& _subPath)
{
	assertThrow(!_subPath.empty(), AssemblyException, "");
	if (_subPath.size() == 1)
	{
		assertThrow(_subPath[0] < m_subs.size(), AssemblyException, "");
		return _subPath[0];
	}

	if (m_subPaths.find(_subPath) == m_subPaths.end())
	{
		size_t objectId = std::numeric_limits<size_t>::max() - m_subPaths.size();
		assertThrow(objectId >= m_subs.size(), AssemblyException, "");
		m_subPaths[_subPath] = objectId;
	}

	return m_subPaths[_subPath];
}

Assembly const* Assembly::subAssemblyById(size_t _subId) const
{
	std::vector<size_t> subIds = decodeSubPath(_subId);
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
