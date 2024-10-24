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
#include <libsolutil/JSON.h>

#include <libsolidity/interface/OptimiserSettings.h>

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
	using TagRefs = std::map<size_t, std::pair<size_t, size_t>>;
	using DataRefs = std::multimap<util::h256, unsigned>;
	using SubAssemblyRefs = std::multimap<size_t, size_t>;
	using ProgramSizeRefs = std::vector<unsigned>;
	using LinkRef = std::pair<size_t, std::string>;

public:
	Assembly(langutil::EVMVersion _evmVersion, bool _creation, std::optional<uint8_t> _eofVersion, std::string _name):
		m_evmVersion(_evmVersion),
		m_creation(_creation),
		m_eofVersion(_eofVersion),
		m_name(std::move(_name))
	{
		// Code section number 0 has to be non-returning.
		m_codeSections.emplace_back(CodeSection{0, 0x80, {}});
	}

	std::optional<uint8_t> eofVersion() const { return m_eofVersion; }
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
	AssemblyItem newAuxDataLoadN(size_t offset);

	AssemblyItem const& append(AssemblyItem _i);
	AssemblyItem const& append(bytes const& _data) { return append(newData(_data)); }

	template <class T> Assembly& operator<<(T const& _d) { append(_d); return *this; }

	/// Pushes the final size of the current assembly itself. Use this when the code is modified
	/// after compilation and CODESIZE is not an option.
	void appendProgramSize() { append(AssemblyItem(PushProgramSize)); }
	void appendLibraryAddress(std::string const& _identifier) { append(newPushLibraryAddress(_identifier)); }
	void appendImmutable(std::string const& _identifier) { append(newPushImmutable(_identifier)); }
	void appendImmutableAssignment(std::string const& _identifier) { append(newImmutableAssignment(_identifier)); }
	void appendAuxDataLoadN(uint16_t _offset) { append(newAuxDataLoadN(_offset));}

	void appendVerbatim(bytes _data, size_t _arguments, size_t _returnVariables)
	{
		append(AssemblyItem(std::move(_data), _arguments, _returnVariables));
	}

	AssemblyItem appendEOFCreate(ContainerID _containerId)
	{
		solAssert(_containerId < m_subs.size(), "EOF Create of undefined container.");
		return append(AssemblyItem::eofCreate(_containerId));
	}
	AssemblyItem appendReturnContract(ContainerID _containerId)
	{
		solAssert(_containerId < m_subs.size(), "Return undefined container ID.");
		return append(AssemblyItem::returnContract(_containerId));
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
	Json assemblyJSON(std::map<std::string, unsigned> const& _sourceIndices, bool _includeSourceList = true) const;

	/// Constructs an @a Assembly from the serialized JSON representation.
	/// @param _json JSON object containing assembly in the format produced by assemblyJSON().
	/// @param _sourceList List of source files the assembly was built from. When the JSON represents
	///     the root assembly, the function will read it from the 'sourceList' field and the parameter
	///     must be empty. It is only used to pass the list down to recursive calls.
	/// @param _level Nesting level of the current assembly in the assembly tree. The root is
	///     at level 0 and the value increases down the tree. Necessary to distinguish between creation
	///     and deployed objects.
	/// @returns Created @a Assembly and the source list read from the 'sourceList' field of the root
	///     assembly or an empty list (in recursive calls).
	static std::pair<std::shared_ptr<Assembly>, std::vector<std::string>> fromJSON(
		Json const& _json,
		std::vector<std::string> const& _sourceList = {},
		size_t _level = 0,
		std::optional<uint8_t> _eofVersion = std::nullopt
	);

	/// Mark this assembly as invalid. Calling ``assemble`` on it will throw.
	void markAsInvalid() { m_invalid = true; }

	std::vector<size_t> decodeSubPath(size_t _subObjectId) const;
	size_t encodeSubPath(std::vector<size_t> const& _subPath);

	bool isCreation() const { return m_creation; }

	struct CodeSection
	{
		uint8_t inputs = 0;
		uint8_t outputs = 0;
		AssemblyItems items{};
	};

	std::vector<CodeSection>& codeSections()
	{
		return m_codeSections;
	}

	std::vector<CodeSection> const& codeSections() const
	{
		return m_codeSections;
	}

protected:
	/// Does the same operations as @a optimise, but should only be applied to a sub and
	/// returns the replaced tags. Also takes an argument containing the tags of this assembly
	/// that are referenced in a super-assembly.
	std::map<u256, u256> const& optimiseInternal(OptimiserSettings const& _settings, std::set<size_t> _tagsReferencedFromOutside);

	/// For EOF and legacy it calculates approximate size of "pure" code without data.
	unsigned codeSize(unsigned subTagSize) const;

	/// Add all assembly items from given JSON array. This function imports the items by iterating through
	/// the code array. This method only works on clean Assembly objects that don't have any items defined yet.
	/// @param _json JSON array that contains assembly items (e.g. json['.code'])
	/// @param _sourceList List of source names.
	void importAssemblyItemsFromJSON(Json const& _code, std::vector<std::string> const& _sourceList);

	/// Creates an AssemblyItem from a given JSON representation.
	/// @param _json JSON object that consists a single assembly item
	/// @param _sourceList List of source names.
	/// @returns AssemblyItem of _json argument.
	AssemblyItem createAssemblyItemFromJSON(Json const& _json, std::vector<std::string> const& _sourceList);

private:
	bool m_invalid = false;

	Assembly const* subAssemblyById(size_t _subId) const;

	void encodeAllPossibleSubPathsInAssemblyTree(std::vector<size_t> _pathFromRoot = {}, std::vector<Assembly*> _assembliesOnPath = {});

	std::shared_ptr<std::string const> sharedSourceName(std::string const& _name) const;

	/// Returns EOF header bytecode | code section sizes offsets | data section size offset
	std::tuple<bytes, std::vector<size_t>, size_t> createEOFHeader(std::set<uint16_t> const& _referencedSubIds) const;

	LinkerObject const& assembleLegacy() const;
	LinkerObject const& assembleEOF() const;

	/// Returns map from m_subs to an index of subcontainer in the final EOF bytecode
	std::map<uint16_t, uint16_t> findReferencedContainers() const;
	/// Returns max AuxDataLoadN offset for the assembly.
	std::optional<uint16_t> findMaxAuxDataLoadNOffset() const;

	/// Assemble bytecode for AssemblyItem type.
	[[nodiscard]] bytes assembleOperation(AssemblyItem const& _item) const;
	[[nodiscard]] bytes assemblePush(AssemblyItem const& _item) const;
	[[nodiscard]] std::pair<bytes, Assembly::LinkRef> assemblePushLibraryAddress(AssemblyItem const& _item, size_t _pos) const;
	[[nodiscard]] bytes assembleVerbatimBytecode(AssemblyItem const& item) const;
	[[nodiscard]] bytes assemblePushDeployTimeAddress() const;
	[[nodiscard]] bytes assembleTag(AssemblyItem const& _item, size_t _pos, bool _addJumpDest) const;

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
	std::map<util::h256, bytes> m_data;
	/// Data that is appended to the very end of the contract.
	bytes m_auxiliaryData;
	std::vector<std::shared_ptr<Assembly>> m_subs;
	std::vector<CodeSection> m_codeSections;
	uint16_t m_currentCodeSection = 0;
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
	std::optional<uint8_t> m_eofVersion;
	/// Internal name of the assembly object, only used with the Yul backend
	/// currently
	std::string m_name;
	langutil::SourceLocation m_currentSourceLocation;

	// FIXME: This being static means that the strings won't be freed when they're no longer needed
	static std::map<std::string, std::shared_ptr<std::string const>> s_sharedSourceNames;

public:
	size_t m_currentModifierDepth = 0;
};

inline std::ostream& operator<<(std::ostream& _out, Assembly const& _a)
{
	_a.assemblyStream(_out);
	return _out;
}

}
