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
			ret += i.bytesRequired(tagSize, m_evmVersion, Precision::Precise);
		if (numberEncodingSize(ret) <= tagSize)
			return static_cast<unsigned>(ret);
	}
}

void Assembly::importAssemblyItemsFromJSON(Json const& _code, std::vector<std::string> const& _sourceList)
{
	solAssert(m_items.empty());
	solRequire(_code.is_array(), AssemblyImportException, "Supplied JSON is not an array.");
	for (auto jsonItemIter = std::begin(_code); jsonItemIter != std::end(_code); ++jsonItemIter)
	{
		AssemblyItem const& newItem = m_items.emplace_back(createAssemblyItemFromJSON(*jsonItemIter, _sourceList));
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

	for (auto const& i: m_items)
		f.feed(i, _debugInfoSelection);
	f.flush();

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
	for (AssemblyItem const& item: m_items)
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
	size_t _level
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

	auto result = std::make_shared<Assembly>(EVMVersion{}, _level == 0 /* _creation */, std::nullopt, "" /* _name */);
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

				auto [subAssembly, emptySourceList] = Assembly::fromJSON(value, _level == 0 ? parsedSourceList : _sourceList, _level + 1);
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
	for (size_t subId = 0; subId < m_subs.size(); ++subId)
	{
		OptimiserSettings settings = _settings;
		Assembly& sub = *m_subs[subId];
		std::map<u256, u256> const& subTagReplacements = sub.optimiseInternal(
			settings,
			JumpdestRemover::referencedTags(m_items, subId)
		);
		// Apply the replacements (can be empty).
		BlockDeduplicator::applyTagReplacement(m_items, subTagReplacements, subId);
	}

	std::map<u256, u256> tagReplacements;
	// Iterate until no new optimisation possibilities are found.
	for (unsigned count = 1; count > 0;)
	{
		count = 0;

		if (_settings.runInliner && !m_eofVersion.has_value())
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
			PeepholeOptimiser peepOpt{m_items, m_evmVersion};
			while (peepOpt.optimise())
			{
				count++;
				assertThrow(count < 64000, OptimizerException, "Peephole optimizer seems to be stuck.");
			}
		}

		// This only modifies PushTags, we have to run again to actually remove code.
		if (_settings.runDeduplicate && !m_eofVersion.has_value())
		{
			BlockDeduplicator deduplicator{m_items};
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
	solAssert(!eof || m_eofVersion == 1, "Invalid EOF version.");

	size_t subTagSize = 1;
	std::map<u256, std::pair<std::string, std::vector<size_t>>> immutableReferencesBySub;
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
	m_tagPositionsInBytecode = std::vector<size_t>(m_usedTags, std::numeric_limits<size_t>::max());
	std::map<size_t, std::pair<size_t, size_t>> tagRef;
	std::multimap<h256, unsigned> dataRef;
	std::multimap<size_t, size_t> subRef;
	std::vector<unsigned> sizeRef; ///< Pointers to code locations where the size of the program is inserted
	unsigned bytesPerTag = numberEncodingSize(bytesRequiredForCode);
	// Adjust bytesPerTag for references to sub assemblies.
	for (AssemblyItem const& i: m_items)
		if (i.type() == PushTag)
		{
			auto [subId, tagId] = i.splitForeignPushTag();
			if (subId == std::numeric_limits<size_t>::max())
				continue;
			assertThrow(subId < m_subs.size(), AssemblyException, "Invalid sub id");
			auto subTagPosition = m_subs[subId]->m_tagPositionsInBytecode.at(tagId);
			assertThrow(subTagPosition != std::numeric_limits<size_t>::max(), AssemblyException, "Reference to tag without position.");
			bytesPerTag = std::max(bytesPerTag, numberEncodingSize(subTagPosition));
		}
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
		if (i.type() != Tag && m_tagPositionsInBytecode[0] == std::numeric_limits<size_t>::max())
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
			assertThrow(!eof, AssemblyException, "PushTag in EOF code");
			ret.bytecode.push_back(tagPush);
			tagRef[ret.bytecode.size()] = i.splitForeignPushTag();
			ret.bytecode.resize(ret.bytecode.size() + bytesPerTag);
			break;
		}
		case PushData:
			ret.bytecode.push_back(dataRefPush);
			dataRef.insert(std::make_pair(h256(i.data()), ret.bytecode.size()));
			ret.bytecode.resize(ret.bytecode.size() + bytesPerDataRef);
			break;
		case PushSub:
			assertThrow(!eof, AssemblyException, "PushSub in EOF code");
			assertThrow(i.data() <= std::numeric_limits<size_t>::max(), AssemblyException, "");
			ret.bytecode.push_back(dataRefPush);
			subRef.insert(std::make_pair(static_cast<size_t>(i.data()), ret.bytecode.size()));
			ret.bytecode.resize(ret.bytecode.size() + bytesPerDataRef);
			break;
		case PushSubSize:
		{
			assertThrow(!eof, AssemblyException, "PushSubSize in EOF code");
			assertThrow(i.data() <= std::numeric_limits<size_t>::max(), AssemblyException, "");
			auto s = subAssemblyById(static_cast<size_t>(i.data()))->assemble().bytecode.size();
			i.setPushedValue(u256(s));
			unsigned b = std::max<unsigned>(1, numberEncodingSize(s));
			ret.bytecode.push_back(static_cast<uint8_t>(pushInstruction(b)));
			ret.bytecode.resize(ret.bytecode.size() + b);
			bytesRef byr(&ret.bytecode.back() + 1 - b, b);
			toBigEndian(s, byr);
			break;
		}
		case PushProgramSize:
		{
			assertThrow(!eof, AssemblyException, "PushProgramSize in EOF code");
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
			assertThrow(!eof, AssemblyException, "PushImmutable in EOF code");
			ret.bytecode.push_back(static_cast<uint8_t>(Instruction::PUSH32));
			// Maps keccak back to the "identifier" std::string of that immutable.
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
			assertThrow(!eof, AssemblyException, "AssignImmutable in EOF code");
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
			assertThrow(i.splitForeignPushTag().first == std::numeric_limits<size_t>::max(), AssemblyException, "Foreign tag.");
			size_t tagId = static_cast<size_t>(i.data());
			assertThrow(ret.bytecode.size() < 0xffffffffL, AssemblyException, "Tag too large.");
			assertThrow(m_tagPositionsInBytecode[tagId] == std::numeric_limits<size_t>::max(), AssemblyException, "Duplicate tag position.");
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

	if (!eof && (!m_subs.empty() || !m_data.empty() || !m_auxiliaryData.empty()))
		// Append an INVALID here to help tests find miscompilation.
		ret.bytecode.push_back(static_cast<uint8_t>(Instruction::INVALID));

	std::map<LinkerObject, size_t> subAssemblyOffsets;
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
		for (auto&& [index, item]: m_items | ranges::views::enumerate)
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
