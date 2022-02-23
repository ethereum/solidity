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
/**
 * @date 2017
 * Abstract assembly interface, subclasses of which are to be used with the generic
 * bytecode generator.
 */

#pragma once

#include <libyul/ASTForward.h>

#include <libsolutil/Common.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/Numeric.h>

#include <functional>
#include <memory>
#include <optional>

namespace solidity::langutil
{
struct SourceLocation;
}

namespace solidity::evmasm
{
enum class Instruction: uint8_t;
}

namespace solidity::yul
{
struct Identifier;

///
/// Assembly class that abstracts both the libevmasm assembly and the new Yul assembly.
///
class AbstractAssembly
{
public:
	using LabelID = size_t;
	using SubID = size_t;
	enum class JumpType { Ordinary, IntoFunction, OutOfFunction };

	virtual ~AbstractAssembly() = default;

	/// Set a new source location valid starting from the next instruction.
	virtual void setSourceLocation(langutil::SourceLocation const& _location) = 0;
	/// Retrieve the current height of the stack. This does not have to be zero
	/// at the beginning.
	virtual int stackHeight() const = 0;
	virtual void setStackHeight(int height) = 0;
	/// Append an EVM instruction.
	virtual void appendInstruction(evmasm::Instruction _instruction) = 0;
	/// Append a constant.
	virtual void appendConstant(u256 const& _constant) = 0;
	/// Append a label.
	virtual void appendLabel(LabelID _labelId) = 0;
	/// Append a label reference.
	virtual void appendLabelReference(LabelID _labelId) = 0;
	/// Generate a new unique label.
	virtual LabelID newLabelId() = 0;
	/// Returns a label identified by the given name. Creates it if it does not yet exist.
	virtual LabelID namedLabel(std::string const& _name, size_t _params, size_t _returns, std::optional<size_t> _sourceID) = 0;
	/// Append a reference to a to-be-linked symbol.
	/// Currently, we assume that the value is always a 20 byte number.
	virtual void appendLinkerSymbol(std::string const& _name) = 0;

	/// Append raw bytes that stay untouched by the optimizer.
	virtual void appendVerbatim(bytes _data, size_t _arguments, size_t _returnVariables) = 0;

	/// Append a jump instruction.
	/// @param _stackDiffAfter the stack adjustment after this instruction.
	/// This is helpful to stack height analysis if there is no continuing control flow.
	virtual void appendJump(int _stackDiffAfter, JumpType _jumpType = JumpType::Ordinary) = 0;

	/// Append a jump-to-immediate operation.
	/// @param _stackDiffAfter the stack adjustment after this instruction.
	virtual void appendJumpTo(LabelID _labelId, int _stackDiffAfter = 0, JumpType _jumpType = JumpType::Ordinary) = 0;
	/// Append a jump-to-if-immediate operation.
	virtual void appendJumpToIf(LabelID _labelId, JumpType _jumpType = JumpType::Ordinary) = 0;

	/// Append the assembled size as a constant.
	virtual void appendAssemblySize() = 0;
	/// Creates a new sub-assembly, which can be referenced using dataSize and dataOffset.
	virtual std::pair<std::shared_ptr<AbstractAssembly>, SubID> createSubAssembly(std::string _name = "") = 0;
	/// Appends the offset of the given sub-assembly or data.
	virtual void appendDataOffset(std::vector<SubID> const& _subPath) = 0;
	/// Appends the size of the given sub-assembly or data.
	virtual void appendDataSize(std::vector<SubID> const& _subPath) = 0;
	/// Appends the given data to the assembly and returns its ID.
	virtual SubID appendData(bytes const& _data) = 0;

	/// Appends loading an immutable variable.
	virtual void appendImmutable(std::string const& _identifier) = 0;
	/// Appends an assignment to an immutable variable.
	virtual void appendImmutableAssignment(std::string const& _identifier) = 0;

	/// Appends data to the very end of the bytecode. Repeated calls concatenate.
	virtual void appendToAuxiliaryData(bytes const& _data) = 0;

	/// Mark this assembly as invalid. Any attempt to request bytecode from it should throw.
	virtual void markAsInvalid() = 0;
};

enum class IdentifierContext { LValue, RValue, VariableDeclaration, NonExternal };

/// Object that is used to resolve references and generate code for access to identifiers external
/// to inline assembly (not used in standalone assembly mode).
struct ExternalIdentifierAccess
{
	using Resolver = std::function<bool(Identifier const&, IdentifierContext, bool /*_crossesFunctionBoundary*/)>;
	/// Resolve an external reference given by the identifier in the given context.
	/// @returns the size of the value (number of stack slots) or size_t(-1) if not found.
	Resolver resolve;
	using CodeGenerator = std::function<void(Identifier const&, IdentifierContext, yul::AbstractAssembly&)>;
	/// Generate code for retrieving the value (rvalue context) or storing the value (lvalue context)
	/// of an identifier. The code should be appended to the assembly. In rvalue context, the value is supposed
	/// to be put onto the stack, in lvalue context, the value is assumed to be at the top of the stack.
	CodeGenerator generateCode;
};

}
