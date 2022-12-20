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
 * Adaptor between AbstractAssembly and libevmasm.
 */

#pragma once

#include <libyul/backends/evm/AbstractAssembly.h>
#include <libyul/AsmAnalysis.h>
#include <liblangutil/SourceLocation.h>

#include <functional>
#include <limits>

namespace solidity::evmasm
{
class Assembly;
class AssemblyItem;
}

namespace solidity::yul
{
class EthAssemblyAdapter: public AbstractAssembly
{
public:
	explicit EthAssemblyAdapter(evmasm::Assembly& _assembly);
	void setSourceLocation(langutil::SourceLocation const& _location) override;
	int stackHeight() const override;
	void setStackHeight(int height) override;
	void appendInstruction(evmasm::Instruction _instruction) override;
	void appendConstant(u256 const& _constant) override;
	void appendLabel(LabelID _labelId) override;
	void appendLabelReference(LabelID _labelId) override;
	size_t newLabelId() override;
	size_t namedLabel(std::string const& _name, size_t _params, size_t _returns, std::optional<size_t> _sourceID) override;
	void appendLinkerSymbol(std::string const& _linkerSymbol) override;
	void appendVerbatim(bytes _data, size_t _arguments, size_t _returnVariables) override;
	void appendJump(int _stackDiffAfter, JumpType _jumpType) override;
	void appendJumpTo(LabelID _labelId, int _stackDiffAfter, JumpType _jumpType) override;
	void appendJumpToIf(LabelID _labelId, JumpType _jumpType) override;
	void appendAssemblySize() override;
	std::pair<std::shared_ptr<AbstractAssembly>, SubID> createSubAssembly(bool _creation, std::optional<uint8_t> _eofVersion, std::string _name = {}) override;
	AbstractAssembly::FunctionID createFunction(uint8_t _args, uint8_t _rets, uint16_t _maxStackHeight) override;
	void setMaxStackHeight(FunctionID _functionID, uint16_t _maxStackHeight) override;
	void beginFunction(AbstractAssembly::FunctionID _functionID) override;
	void endFunction() override;
	void appendFunctionCall(FunctionID _functionID) override;
	void appendFunctionReturn() override;
	void appendDataOffset(std::vector<SubID> const& _subPath) override;
	void appendDataSize(std::vector<SubID> const& _subPath) override;
	SubID appendData(bytes const& _data) override;

	void appendToAuxiliaryData(bytes const& _data) override;

	void appendImmutable(std::string const& _identifier) override;
	void appendImmutableAssignment(std::string const& _identifier) override;

	void markAsInvalid() override;

private:
	static LabelID assemblyTagToIdentifier(evmasm::AssemblyItem const& _tag);
	void appendJumpInstruction(evmasm::Instruction _instruction, JumpType _jumpType);

	evmasm::Assembly& m_assembly;
	std::map<SubID, u256> m_dataHashBySubId;
	size_t m_nextDataCounter = std::numeric_limits<size_t>::max() / 2;
};
}
