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
 * Helper to compile Yul code using libevmasm.
 */

#pragma once

#include <libyul/backends/evm/AbstractAssembly.h>
#include <libyul/AsmAnalysis.h>
#include <liblangutil/EVMVersion.h>

namespace solidity::evmasm
{
class Assembly;
}

namespace solidity::yul
{
struct Block;
struct AsmAnalysisInfo;

class CodeGenerator
{
public:
	/// Performs code generation and appends generated to _assembly.
	static void assemble(
		Block const& _parsedData,
		AsmAnalysisInfo& _analysisInfo,
		evmasm::Assembly& _assembly,
		langutil::EVMVersion _evmVersion,
		ExternalIdentifierAccess::CodeGenerator _identifierAccess = {},
		bool _system = false,
		bool _optimizeStackAllocation = false
	);
};
}
