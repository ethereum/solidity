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
 * Assembly interface that ignores everything. Can be used as a backend for a compilation dry-run.
 */

#pragma once

#include <libyul/backends/evm/AbstractAssembly.h>

#include <libyul/backends/evm/EVMDialect.h>

#include <libevmasm/LinkerObject.h>

#include <map>

namespace langutil
{
struct SourceLocation;
}

namespace yul
{


/**
 * Assembly class that just ignores everything and only performs stack counting.
 * The purpose is to use this assembly for compilation dry-runs.
 */
class NoOutputAssembly: public AbstractAssembly
{
public:
	explicit NoOutputAssembly(bool _evm15 = false): m_evm15(_evm15) { }
	virtual ~NoOutputAssembly() = default;

	void setSourceLocation(langutil::SourceLocation const&) override {}
	int stackHeight() const override { return m_stackHeight; }
	void appendInstruction(dev::eth::Instruction _instruction) override;
	void appendConstant(dev::u256 const& _constant) override;
	void appendLabel(LabelID _labelId) override;
	void appendLabelReference(LabelID _labelId) override;
	LabelID newLabelId() override;
	LabelID namedLabel(std::string const& _name) override;
	void appendLinkerSymbol(std::string const& _name) override;

	void appendJump(int _stackDiffAfter) override;
	void appendJumpTo(LabelID _labelId, int _stackDiffAfter) override;
	void appendJumpToIf(LabelID _labelId) override;
	void appendBeginsub(LabelID _labelId, int _arguments) override;
	void appendJumpsub(LabelID _labelId, int _arguments, int _returns) override;
	void appendReturnsub(int _returns, int _stackDiffAfter) override;

	void appendAssemblySize() override;
	std::pair<std::shared_ptr<AbstractAssembly>, SubID> createSubAssembly() override;
	void appendDataOffset(SubID _sub) override;
	void appendDataSize(SubID _sub) override;
	SubID appendData(dev::bytes const& _data) override;

private:
	bool m_evm15 = false; ///< if true, switch to evm1.5 mode
	int m_stackHeight = 0;
};


/**
 * EVM dialect that does not generate any code.
 */
struct NoOutputEVMDialect: public EVMDialect
{
	explicit NoOutputEVMDialect(std::shared_ptr<EVMDialect const> const& _copyFrom);
};


}
