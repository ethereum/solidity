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
 * Assembly interface that ignores everything. Can be used as a backend for a compilation dry-run.
 */

#pragma once

#include <libyul/backends/evm/AbstractAssembly.h>

#include <libyul/backends/evm/EVMDialect.h>

#include <libevmasm/LinkerObject.h>

#include <map>

namespace solidity::langutil
{
struct SourceLocation;
}

namespace solidity::yul
{


/**
 * Assembly class that just ignores everything and only performs stack counting.
 * The purpose is to use this assembly for compilation dry-runs.
 */
class NoOutputAssembly: public AbstractAssembly
{
public:
	explicit NoOutputAssembly(langutil::EVMVersion _evmVersion): m_evmVersion(_evmVersion) { }
	~NoOutputAssembly() override = default;

	void setSourceLocation(langutil::SourceLocation const&) override {}
	int stackHeight() const override { return m_stackHeight; }
	void setStackHeight(int height) override { m_stackHeight = height; }
	void appendInstruction(evmasm::Instruction _instruction) override;
	void appendConstant(u256 const& _constant) override;
	void appendLabel(LabelID _labelId) override;
	void appendLabelReference(LabelID _labelId) override;
	LabelID newLabelId() override;
	LabelID namedLabel(std::string const& _name, size_t _params, size_t _returns, std::optional<size_t> _sourceID) override;
	void appendLinkerSymbol(std::string const& _name) override;
	void appendVerbatim(bytes _data, size_t _arguments, size_t _returnVariables) override;

	void appendJump(int _stackDiffAfter, JumpType _jumpType) override;
	void appendJumpTo(LabelID _labelId, int _stackDiffAfter, JumpType _jumpType) override;
	void appendJumpToIf(LabelID _labelId, JumpType _jumpType) override;

	void appendAssemblySize() override;
	std::pair<std::shared_ptr<AbstractAssembly>, SubID> createSubAssembly(bool _creation, std::string _name = "") override;
	void appendDataOffset(std::vector<SubID> const& _subPath) override;
	void appendDataSize(std::vector<SubID> const& _subPath) override;
	SubID appendData(bytes const& _data) override;

	void appendToAuxiliaryData(bytes const&) override {}

	void appendImmutable(std::string const& _identifier) override;
	void appendImmutableAssignment(std::string const& _identifier) override;

	void markAsInvalid() override {}

private:
	int m_stackHeight = 0;
	langutil::EVMVersion m_evmVersion;
};


/**
 * EVM dialect that does not generate any code.
 */
struct NoOutputEVMDialect: public EVMDialect
{
	explicit NoOutputEVMDialect(EVMDialect const& _copyFrom);
};


}
