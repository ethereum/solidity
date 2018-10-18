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

#pragma once

#include <libevmasm/Instruction.h>
#include <libevmasm/SourceLocation.h>
#include <libevmasm/AssemblyItem.h>
#include <libevmasm/LinkerObject.h>
#include <libevmasm/Exceptions.h>

#include <libsolidity/interface/EVMVersion.h>

#include <libdevcore/Common.h>
#include <libdevcore/Assertions.h>
#include <libdevcore/Keccak256.h>

#include <json/json.h>

#include <iostream>
#include <sstream>
#include <memory>

namespace dev
{
namespace eth
{

using AssemblyPointer = std::shared_ptr<Assembly>;

class Assembly
{
public:
	Assembly() {}

	AssemblyItem newTag() { assertThrow(m_usedTags < 0xffffffff, AssemblyException, ""); return AssemblyItem(Tag, m_usedTags++); }
	AssemblyItem newPushTag() { assertThrow(m_usedTags < 0xffffffff, AssemblyException, ""); return AssemblyItem(PushTag, m_usedTags++); }
	/// Returns a tag identified by the given name. Creates it if it does not yet exist.
	AssemblyItem namedTag(std::string const& _name);
	AssemblyItem newData(bytes const& _data) { h256 h(dev::keccak256(asString(_data))); m_data[h] = _data; return AssemblyItem(PushData, h); }
	bytes const& data(h256 const& _i) const { return m_data.at(_i); }
	AssemblyItem newSub(AssemblyPointer const& _sub) { m_subs.push_back(_sub); return AssemblyItem(PushSub, m_subs.size() - 1); }
	Assembly const& sub(size_t _sub) const { return *m_subs.at(_sub); }
	Assembly& sub(size_t _sub) { return *m_subs.at(_sub); }
	AssemblyItem newPushSubSize(u256 const& _subId) { return AssemblyItem(PushSubSize, _subId); }
	AssemblyItem newPushLibraryAddress(std::string const& _identifier);

	AssemblyItem const& append(AssemblyItem const& _i);
	AssemblyItem const& append(std::string const& _data) { return append(newPushString(_data)); }
	AssemblyItem const& append(bytes const& _data) { return append(newData(_data)); }

	template <class T> Assembly& operator<<(T const& _d) { append(_d); return *this; }

	/// Pushes the final size of the current assembly itself. Use this when the code is modified
	/// after compilation and CODESIZE is not an option.
	void appendProgramSize() { append(AssemblyItem(PushProgramSize)); }
	void appendLibraryAddress(std::string const& _identifier) { append(newPushLibraryAddress(_identifier)); }

	AssemblyItem appendJump() { auto ret = append(newPushTag()); append(solidity::Instruction::JUMP); return ret; }
	AssemblyItem appendJumpI() { auto ret = append(newPushTag()); append(solidity::Instruction::JUMPI); return ret; }
	AssemblyItem appendJump(AssemblyItem const& _tag) { auto ret = append(_tag.pushTag()); append(solidity::Instruction::JUMP); return ret; }
	AssemblyItem appendJumpI(AssemblyItem const& _tag) { auto ret = append(_tag.pushTag()); append(solidity::Instruction::JUMPI); return ret; }

	/// Adds a subroutine to the code (in the data section) and pushes its size (via a tag)
	/// on the stack. @returns the pushsub assembly item.
	AssemblyItem appendSubroutine(AssemblyPointer const& _assembly) { auto sub = newSub(_assembly); append(newPushSubSize(size_t(sub.data()))); return sub; }
	void pushSubroutineSize(size_t _subRoutine) { append(newPushSubSize(_subRoutine)); }
	/// Pushes the offset of the subroutine.
	void pushSubroutineOffset(size_t _subRoutine) { append(AssemblyItem(PushSub, _subRoutine)); }

	/// Appends @a _data literally to the very end of the bytecode.
	void appendAuxiliaryDataToEnd(bytes const& _data) { m_auxiliaryData += _data; }

	/// Returns the assembly items.
	AssemblyItems const& items() const { return m_items; }

	int deposit() const { return m_deposit; }
	void adjustDeposit(int _adjustment) { m_deposit += _adjustment; assertThrow(m_deposit >= 0, InvalidDeposit, ""); }
	void setDeposit(int _deposit) { m_deposit = _deposit; assertThrow(m_deposit >= 0, InvalidDeposit, ""); }

	/// Changes the source location used for each appended item.
	void setSourceLocation(SourceLocation const& _location) { m_currentSourceLocation = _location; }

	/// Assembles the assembly into bytecode. The assembly should not be modified after this call, since the assembled version is cached.
	LinkerObject const& assemble() const;

	struct OptimiserSettings
	{
		bool isCreation = false;
		bool runJumpdestRemover = false;
		bool runPeephole = false;
		bool runDeduplicate = false;
		bool runCSE = false;
		bool runConstantOptimiser = false;
		solidity::EVMVersion evmVersion;
		/// This specifies an estimate on how often each opcode in this assembly will be executed,
		/// i.e. use a small value to optimise for size and a large value to optimise for runtime gas usage.
		size_t expectedExecutionsPerDeployment = 200;
	};

	/// Execute optimisation passes as defined by @a _settings and return the optimised assembly.
	Assembly& optimise(OptimiserSettings const& _settings);

	/// Modify (if @a _enable is set) and return the current assembly such that creation and
	/// execution gas usage is optimised. @a _isCreation should be true for the top-level assembly.
	/// @a _runs specifes an estimate on how often each opcode in this assembly will be executed,
	/// i.e. use a small value to optimise for size and a large value to optimise for runtime.
	/// If @a _enable is not set, will perform some simple peephole optimizations.
	Assembly& optimise(bool _enable, EVMVersion _evmVersion, bool _isCreation = true, size_t _runs = 200);

	/// Create a text representation of the assembly.
	std::string assemblyString(
		StringMap const& _sourceCodes = StringMap()
	) const;
	void assemblyStream(
		std::ostream& _out,
		std::string const& _prefix = "",
		StringMap const& _sourceCodes = StringMap()
	) const;

	/// Create a JSON representation of the assembly.
	Json::Value assemblyJSON(
		StringMap const& _sourceCodes = StringMap()
	) const;

public:
	// These features are only used by LLL
	AssemblyItem newPushString(std::string const& _data) { h256 h(dev::keccak256(_data)); m_strings[h] = _data; return AssemblyItem(PushString, h); }

	void append(Assembly const& _a);
	void append(Assembly const& _a, int _deposit);

	void injectStart(AssemblyItem const& _i);

	AssemblyItem const& back() const { return m_items.back(); }
	std::string backString() const { return m_items.size() && m_items.back().type() == PushString ? m_strings.at((h256)m_items.back().data()) : std::string(); }

protected:
	/// Does the same operations as @a optimise, but should only be applied to a sub and
	/// returns the replaced tags. Also takes an argument containing the tags of this assembly
	/// that are referenced in a super-assembly.
	std::map<u256, u256> optimiseInternal(OptimiserSettings const& _settings, std::set<size_t> const& _tagsReferencedFromOutside);

	unsigned bytesRequired(unsigned subTagSize) const;

private:
	static Json::Value createJsonValue(std::string _name, int _begin, int _end, std::string _value = std::string(), std::string _jumpType = std::string());
	static std::string toStringInHex(u256 _value);

protected:
	/// 0 is reserved for exception
	unsigned m_usedTags = 1;
	std::map<std::string, size_t> m_namedTags;
	AssemblyItems m_items;
	std::map<h256, bytes> m_data;
	/// Data that is appended to the very end of the contract.
	bytes m_auxiliaryData;
	std::vector<std::shared_ptr<Assembly>> m_subs;
	std::map<h256, std::string> m_strings;
	std::map<h256, std::string> m_libraries; ///< Identifiers of libraries to be linked.

	mutable LinkerObject m_assembledObject;
	mutable std::vector<size_t> m_tagPositionsInBytecode;

	int m_deposit = 0;

	SourceLocation m_currentSourceLocation;
};

inline std::ostream& operator<<(std::ostream& _out, Assembly const& _a)
{
	_a.assemblyStream(_out);
	return _out;
}

}
}
