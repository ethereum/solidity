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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity compiler.
 */

#include <libsolidity/codegen/Compiler.h>
#include <libevmasm/Assembly.h>
#include <libsolidity/codegen/ContractCompiler.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

void Compiler::compileContract(
	ContractDefinition const& _contract,
	std::map<const ContractDefinition*, eth::Assembly const*> const& _contracts,
	bytes const& _metadata
)
{
	ContractCompiler runtimeCompiler(nullptr, m_runtimeContext, m_optimize, m_optimizeRuns);
	runtimeCompiler.compileContract(_contract, _contracts);
	m_runtimeContext.appendAuxiliaryData(_metadata);

	// This might modify m_runtimeContext because it can access runtime functions at
	// creation time.
	ContractCompiler creationCompiler(&runtimeCompiler, m_context, m_optimize, 1);
	m_runtimeSub = creationCompiler.compileConstructor(_contract, _contracts);

	m_context.optimise(m_optimize, m_optimizeRuns);
}

eth::AssemblyItem Compiler::functionEntryLabel(FunctionDefinition const& _function) const
{
	return m_runtimeContext.functionEntryLabelIfExists(_function);
}
