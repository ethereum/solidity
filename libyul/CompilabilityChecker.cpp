// SPDX-License-Identifier: GPL-3.0
/**
 * Component that checks whether all variables are reachable on the stack.
 */

#include <libyul/CompilabilityChecker.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>

#include <libyul/backends/evm/EVMCodeTransform.h>
#include <libyul/backends/evm/NoOutputAssembly.h>

#include <liblangutil/EVMVersion.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

map<YulString, int> CompilabilityChecker::run(
	Dialect const& _dialect,
	Object const& _object,
	bool _optimizeStackAllocation
)
{
	if (EVMDialect const* evmDialect = dynamic_cast<EVMDialect const*>(&_dialect))
	{
		NoOutputEVMDialect noOutputDialect(*evmDialect);

		yul::AsmAnalysisInfo analysisInfo =
			yul::AsmAnalyzer::analyzeStrictAssertCorrect(noOutputDialect, _object);

		BuiltinContext builtinContext;
		builtinContext.currentObject = &_object;
		for (auto name: _object.dataNames())
			builtinContext.subIDs[name] = 1;
		NoOutputAssembly assembly;
		CodeTransform transform(
			assembly,
			analysisInfo,
			*_object.code,
			noOutputDialect,
			builtinContext,
			_optimizeStackAllocation
		);
		try
		{
			transform(*_object.code);
		}
		catch (StackTooDeepError const&)
		{
			yulAssert(!transform.stackErrors().empty(), "Got stack too deep exception that was not stored.");
		}

		std::map<YulString, int> functions;
		for (StackTooDeepError const& error: transform.stackErrors())
			functions[error.functionName] = max(error.depth, functions[error.functionName]);

		return functions;
	}
	else
		return {};
}
