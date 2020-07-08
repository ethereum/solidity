// SPDX-License-Identifier: GPL-3.0
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity compiler.
 */

#include <libsolidity/codegen/Compiler.h>

#include <libsolidity/codegen/ContractCompiler.h>
#include <libevmasm/Assembly.h>

using namespace std;
using namespace solidity;
using namespace solidity::frontend;

void Compiler::compileContract(
	ContractDefinition const& _contract,
	std::map<ContractDefinition const*, shared_ptr<Compiler const>> const& _otherCompilers,
	bytes const& _metadata
)
{
	ContractCompiler runtimeCompiler(nullptr, m_runtimeContext, m_optimiserSettings);
	runtimeCompiler.compileContract(_contract, _otherCompilers);
	m_runtimeContext.appendAuxiliaryData(_metadata);

	// This might modify m_runtimeContext because it can access runtime functions at
	// creation time.
	OptimiserSettings creationSettings{m_optimiserSettings};
	// The creation code will be executed at most once, so we modify the optimizer
	// settings accordingly.
	creationSettings.expectedExecutionsPerDeployment = 1;
	ContractCompiler creationCompiler(&runtimeCompiler, m_context, creationSettings);
	m_runtimeSub = creationCompiler.compileConstructor(_contract, _otherCompilers);

	m_context.optimise(m_optimiserSettings);

	solAssert(m_context.requestedYulFunctionsRan(), "requestedYulFunctions() was not called.");
	solAssert(m_runtimeContext.requestedYulFunctionsRan(), "requestedYulFunctions() was not called.");
}

std::shared_ptr<evmasm::Assembly> Compiler::runtimeAssemblyPtr() const
{
	solAssert(m_context.runtimeContext(), "");
	return m_context.runtimeContext()->assemblyPtr();
}

evmasm::AssemblyItem Compiler::functionEntryLabel(FunctionDefinition const& _function) const
{
	return m_runtimeContext.functionEntryLabelIfExists(_function);
}
