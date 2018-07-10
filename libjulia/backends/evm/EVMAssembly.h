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
/**
 * Assembly interface for EVM and EVM1.5.
 */

#pragma once

#include <libjulia/backends/evm/AbstractAssembly.h>

#include <libevmasm/LinkerObject.h>

#include <map>

namespace dev
{
namespace julia
{

class EVMAssembly: public AbstractAssembly
{
public:
	explicit EVMAssembly(bool _evm15 = false): m_evm15(_evm15) { }
	virtual ~EVMAssembly() {}

	/// Set a new source location valid starting from the next instruction.
	virtual void setSourceLocation(SourceLocation const& _location) override;
	/// Retrieve the current height of the stack. This does not have to be zero
	/// at the beginning.
	virtual int stackHeight() const override { return m_stackHeight; }
	/// Append an EVM instruction.
	virtual void appendInstruction(solidity::Instruction _instruction) override;
	/// Append a constant.
	virtual void appendConstant(u256 const& _constant) override;
	/// Append a label.
	virtual void appendLabel(LabelID _labelId) override;
	/// Append a label reference.
	virtual void appendLabelReference(LabelID _labelId) override;
	/// Generate a new unique label.
	virtual LabelID newLabelId() override;
	/// Returns a label identified by the given name. Creates it if it does not yet exist.
	virtual LabelID namedLabel(std::string const& _name) override;
	/// Append a reference to a to-be-linked symbol.
	/// Currently, we assume that the value is always a 20 byte number.
	virtual void appendLinkerSymbol(std::string const& _name) override;

	/// Append a jump instruction.
	/// @param _stackDiffAfter the stack adjustment after this instruction.
	virtual void appendJump(int _stackDiffAfter) override;
	/// Append a jump-to-immediate operation.
	virtual void appendJumpTo(LabelID _labelId, int _stackDiffAfter) override;
	/// Append a jump-to-if-immediate operation.
	virtual void appendJumpToIf(LabelID _labelId) override;
	/// Start a subroutine.
	virtual void appendBeginsub(LabelID _labelId, int _arguments) override;
	/// Call a subroutine.
	virtual void appendJumpsub(LabelID _labelId, int _arguments, int _returns) override;
	/// Return from a subroutine.
	virtual void appendReturnsub(int _returns, int _stackDiffAfter) override;

	/// Append the assembled size as a constant.
	virtual void appendAssemblySize() override;

	/// Resolves references inside the bytecode and returns the linker object.
	eth::LinkerObject finalize();

private:
	void setLabelToCurrentPosition(AbstractAssembly::LabelID _labelId);
	void appendLabelReferenceInternal(AbstractAssembly::LabelID _labelId);
	void updateReference(size_t pos, size_t size, u256 value);

	bool m_evm15 = false; ///< if true, switch to evm1.5 mode
	LabelID m_nextLabelId = 0;
	int m_stackHeight = 0;
	bytes m_bytecode;
	std::map<std::string, LabelID> m_namedLabels;
	std::map<LabelID, size_t> m_labelPositions;
	std::map<size_t, LabelID> m_labelReferences;
	std::vector<size_t> m_assemblySizePositions;
};

}
}
