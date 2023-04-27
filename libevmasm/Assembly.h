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

#pragma once

#include <libevmasm/Instruction.h>
#include <liblangutil/SourceLocation.h>
#include <libevmasm/AssemblyItem.h>
#include <libevmasm/LinkerObject.h>
#include <libevmasm/Exceptions.h>

#include <liblangutil/DebugInfoSelection.h>
#include <liblangutil/EVMVersion.h>

#include <libsolutil/Common.h>
#include <libsolutil/Assertions.h>
#include <libsolutil/Keccak256.h>

#include <libsolidity/interface/OptimiserSettings.h>

#include <json/json.h>

#include <iostream>
#include <sstream>
#include <memory>
#include <map>
#include <utility>

namespace solidity::evmasm
{

using AssemblyPointer = std::shared_ptr<Assembly>;

class Assembly
{
public:
	Assembly(langutil::EVMVersion _evmVersion, bool _creation, std::string _name): m_evmVersion(_evmVersion), m_creation(_creation), m_name(std::move(_name)) { }

	AssemblyItem newTag() { assertThrow(m_usedTags < 0xffffffff, AssemblyException, ""); return AssemblyItem(Tag, m_usedTags++); }
	AssemblyItem newPushTag() { assertThrow(m_usedTags < 0xffffffff, AssemblyException, ""); return AssemblyItem(PushTag, m_usedTags++); }
	/// Returns a tag identified by the given name. Creates it if it does not yet exist.
	AssemblyItem namedTag(std::string const& _name, size_t _params, size_t _returns, std::optional<uint64_t> _sourceID);
	AssemblyItem newData(bytes const& _data) { util::h256 h(util::keccak256(util::asString(_data))); m_data[h] = _data; return AssemblyItem(PushData, h); }
	bytes const& data(util::h256 const& _i) const { return m_data.at(_i); }
	AssemblyItem newSub(AssemblyPointer const& _sub) { m_subs.push_back(_sub); return AssemblyItem(PushSub, m_subs.size() - 1); }
	Assembly const& sub(size_t _sub) const { return *m_subs.at(_sub); }
	Assembly& sub(size_t _sub) { return *m_subs.at(_sub); }
	size_t numSubs() const { return m_subs.size(); }
	AssemblyItem newPushSubSize(u256 const& _subId) { return AssemblyItem(PushSubSize, _subId); }
	AssemblyItem newPushLibraryAddress(std::string const& _identifier);
	AssemblyItem newPushImmutable(std::string const& _identifier);
	AssemblyItem newImmutableAssignment(std::string const& _identifier);

	AssemblyItem const& append(AssemblyItem _i);
	AssemblyItem const& append(bytes const& _data) { return append(newData(_data)); }

	template <class T> Assembly& operator<<(T const& _d) { append(_d); return *this; }

	/// Pushes the final size of the current assembly itself. Use this when the code is modified
	/// after compilation and CODESIZE is not an option.
	void appendProgramSize() { append(AssemblyItem(PushProgramSize)); }
	void appendLibraryAddress(std::string const& _identifier) { append(newPushLibraryAddress(_identifier)); }
	void appendImmutable(std::string const& _identifier) { append(newPushImmutable(_identifier)); }
	void appendImmutableAssignment(std::string const& _identifier) { append(newImmutableAssignment(_identifier)); }

	void appendVerbatim(bytes _data, size_t _arguments, size_t _returnVariables)
	{
		append(AssemblyItem(std::move(_data), _arguments, _returnVariables));
	}

	AssemblyItem appendJump() { auto ret = append(newPushTag()); append(Instruction::JUMP); return ret; }
	AssemblyItem appendJumpI() { auto ret = append(newPushTag()); append(Instruction::JUMPI); return ret; }
	AssemblyItem appendJump(AssemblyItem const& _tag) { auto ret = append(_tag.pushTag()); append(Instruction::JUMP); return ret; }
	AssemblyItem appendJumpI(AssemblyItem const& _tag) { auto ret = append(_tag.pushTag()); append(Instruction::JUMPI); return ret; }

	/// Adds a subroutine to the code (in the data section) and pushes its size (via a tag)
	/// on the stack. @returns the pushsub assembly item.
	AssemblyItem appendSubroutine(AssemblyPointer const& _assembly) { auto sub = newSub(_assembly); append(newPushSubSize(size_t(sub.data()))); return sub; }
	void pushSubroutineSize(size_t _subRoutine) { append(newPushSubSize(_subRoutine)); }
	/// Pushes the offset of the subroutine.
	void pushSubroutineOffset(size_t _subRoutine) { append(AssemblyItem(PushSub, _subRoutine)); }

	/// Appends @a _data literally to the very end of the bytecode.
	void appendToAuxiliaryData(bytes const& _data) { m_auxiliaryData += _data; }

	/// Returns the assembly items.
	AssemblyItems const& items() const { return m_items; }

	/// Returns the mutable assembly items. Use with care!
	AssemblyItems& items() { return m_items; }

	int deposit() const { return m_deposit; }
	void adjustDeposit(int _adjustment) { m_deposit += _adjustment; assertThrow(m_deposit >= 0, InvalidDeposit, ""); }
	void setDeposit(int _deposit) { m_deposit = _deposit; assertThrow(m_deposit >= 0, InvalidDeposit, ""); }
	std::string const& name() const { return m_name; }

	/// Changes the source location used for each appended item.
	void setSourceLocation(langutil::SourceLocation const& _location) { m_currentSourceLocation = _location; }
	langutil::SourceLocation const& currentSourceLocation() const { return m_currentSourceLocation; }
	langutil::EVMVersion const& evmVersion() const { return m_evmVersion; }

	/// Assembles the assembly into bytecode. The assembly should not be modified after this call, since the assembled version is cached.
	LinkerObject const& assemble() const;

	struct OptimiserSettings
	{
		bool runInliner = false;
		bool runJumpdestRemover = false;
		bool runPeephole = false;
		bool runDeduplicate = false;
		bool runCSE = false;
		bool runConstantOptimiser = false;
		langutil::EVMVersion evmVersion;
		/// This specifies an estimate on how often each opcode in this assembly will be executed,
		/// i.e. use a small value to optimise for size and a large value to optimise for runtime gas usage.
		size_t expectedExecutionsPerDeployment = frontend::OptimiserSettings{}.expectedExecutionsPerDeployment;

		static OptimiserSettings translateSettings(frontend::OptimiserSettings const& _settings, langutil::EVMVersion const& _evmVersion);
	};

	/// Modify and return the current assembly such that creation and execution gas usage
	/// is optimised according to the settings in @a _settings.
	Assembly& optimise(OptimiserSettings const& _settings);

	/// Create a text representation of the assembly.
	std::string assemblyString(
		langutil::DebugInfoSelection const& _debugInfoSelection = langutil::DebugInfoSelection::Default(),
		StringMap const& _sourceCodes = StringMap()
	) const;
	void assemblyStream(
		std::ostream& _out,
		langutil::DebugInfoSelection const& _debugInfoSelection = langutil::DebugInfoSelection::Default(),
		std::string const& _prefix = "",
		StringMap const& _sourceCodes = StringMap()
	) const;

	/// Create a JSON representation of the assembly.
	Json::Value assemblyJSON(std::vector<std::string> const& _sources, bool _includeSourceList = true) const;

	/// Loads the JSON representation of assembly.
	/// @param _json JSON object containing assembly in the format produced by assemblyJSON().
	/// @param _sourceList list of source names (used to supply the source-list from the root-assembly object to sub-assemblies).
	/// @param _level this function might be called recursively, _level reflects the nesting level.
	/// @returns Resulting Assembly object loaded from given json.
	static std::pair<std::shared_ptr<Assembly>, std::vector<std::string>> fromJSON(
		Json::Value const& _json,
		std::vector<std::string> const& _sourceList = {},
		int _level = 0
	);

	/// Mark this assembly as invalid. Calling ``assemble`` on it will throw.
	void markAsInvalid() { m_invalid = true; }

	std::vector<size_t> decodeSubPath(size_t _subObjectId) const;
	size_t encodeSubPath(std::vector<size_t> const& _subPath);

	bool isCreation() const { return m_creation; }

protected:
	/// Does the same operations as @a optimise, but should only be applied to a sub and
	/// returns the replaced tags. Also takes an argument containing the tags of this assembly
	/// that are referenced in a super-assembly.
	std::map<u256, u256> const& optimiseInternal(OptimiserSettings const& _settings, std::set<size_t> _tagsReferencedFromOutside);

	unsigned codeSize(unsigned subTagSize) const;

	/// Add all assembly items from given JSON array. This function imports the items by iterating through
	/// the code array. This method only works on clean Assembly objects that don't have any items defined yet.
	/// @param _json JSON array that contains assembly items (e.g. json['.code'])
	/// @param _sourceList list of source names.
	void importAssemblyItemsFromJSON(Json::Value const& _code, std::vector<std::string> const& _sourceList);

	/// Creates an AssemblyItem from a given JSON representation.
	/// @param _json JSON object that consists a single assembly item
	/// @param _sourceList list of source names.
	/// @returns AssemblyItem of _json argument.
	AssemblyItem createAssemblyItemFromJSON(Json::Value const& _json, std::vector<std::string> const& _sourceList);

private:
	bool m_invalid = false;

	Assembly const* subAssemblyById(size_t _subId) const;

protected:
	/// 0 is reserved for exception
	unsigned m_usedTags = 1;

	struct NamedTagInfo
	{
		size_t id;
		std::optional<size_t> sourceID;
		size_t params;
		size_t returns;
	};

	std::map<std::string, NamedTagInfo> m_namedTags;
	AssemblyItems m_items;
	std::map<util::h256, bytes> m_data;
	/// Data that is appended to the very end of the contract.
	bytes m_auxiliaryData;
	std::vector<std::shared_ptr<Assembly>> m_subs;
	std::map<util::h256, std::string> m_strings;
	std::map<util::h256, std::string> m_libraries; ///< Identifiers of libraries to be linked.
	std::map<util::h256, std::string> m_immutables; ///< Identifiers of immutables.

	/// Map from a vector representing a path to a particular sub assembly to sub assembly id.
	/// This map is used only for sub-assemblies which are not direct sub-assemblies (where path is having more than one value).
	std::map<std::vector<size_t>, size_t> m_subPaths;

	/// Contains the tag replacements relevant for super-assemblies.
	/// If set, it means the optimizer has run and we will not run it again.
	std::optional<std::map<u256, u256>> m_tagReplacements;

	mutable LinkerObject m_assembledObject;
	mutable std::vector<size_t> m_tagPositionsInBytecode;

	langutil::EVMVersion m_evmVersion;

	int m_deposit = 0;
	/// True, if the assembly contains contract creation code.
	bool const m_creation = false;
	/// Internal name of the assembly object, only used with the Yul backend
	/// currently
	std::string m_name;
	langutil::SourceLocation m_currentSourceLocation;

public:
	size_t m_currentModifierDepth = 0;
};

inline std::ostream& operator<<(std::ostream& _out, Assembly const& _a)
{
	_a.assemblyStream(_out);
	return _out;
}

}
