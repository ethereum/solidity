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

#include <libyul/backends/evm/AsmCodeGen.h>

#include <libyul/backends/evm/EthAssemblyAdapter.h>
#include <libyul/backends/evm/EVMCodeTransform.h>
#include <libyul/backends/evm/OptimizedEVMCodeTransform.h>
#include <libyul/AST.h>
#include <libyul/AsmAnalysisInfo.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::langutil;

void CodeGenerator::assemble(
	Block const& _parsedData,
	AsmAnalysisInfo& _analysisInfo,
	evmasm::Assembly& _assembly,
	langutil::EVMVersion _evmVersion,
	ExternalIdentifierAccess::CodeGenerator _identifierAccessCodeGen,
	bool _system,
	bool _optimizeStackAllocation
)
{
	EthAssemblyAdapter assemblyAdapter(_assembly);
	BuiltinContext builtinContext;
	if (_system && _optimizeStackAllocation && _evmVersion > EVMVersion::homestead())
	{
		int oldStackHeight = assemblyAdapter.stackHeight();
		assemblyAdapter.setStackHeight(0);
		auto stackErrors = OptimizedEVMCodeTransform::run(assemblyAdapter, _analysisInfo, _parsedData, EVMDialect::strictAssemblyForEVM(_evmVersion), builtinContext, _system);
		assemblyAdapter.setStackHeight(oldStackHeight);
		if (!stackErrors.empty())
			assertThrow(
				false,
				langutil::StackTooDeepError,
				"Stack too deep when compiling inline assembly" +
				(stackErrors.front().comment() ? ": " + *stackErrors.front().comment() : ".")
			);
	}
	else
	{
		CodeTransform transform(
			assemblyAdapter,
			_analysisInfo,
			_parsedData,
			EVMDialect::strictAssemblyForEVM(_evmVersion),
			builtinContext,
			_optimizeStackAllocation,
			_identifierAccessCodeGen,
			_system
		);
		transform(_parsedData);
		if (!transform.stackErrors().empty())
			assertThrow(
				false,
				langutil::StackTooDeepError,
				"Stack too deep when compiling inline assembly" +
				(transform.stackErrors().front().comment() ? ": " + *transform.stackErrors().front().comment() : ".")
			);
	}
}
