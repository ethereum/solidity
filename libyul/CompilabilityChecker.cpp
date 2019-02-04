/*(
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
 * Component that checks whether all variables are reachable on the stack.
 */

#include <libyul/CompilabilityChecker.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>

#include <libyul/backends/evm/EVMCodeTransform.h>
#include <libyul/backends/evm/NoOutputAssembly.h>

#include <liblangutil/EVMVersion.h>

using namespace std;
using namespace yul;
using namespace dev;
using namespace dev::solidity;

std::map<YulString, int> CompilabilityChecker::run(std::shared_ptr<Dialect> _dialect, Block const& _ast)
{
	if (_dialect->flavour == AsmFlavour::Yul)
		return {};

	solAssert(_dialect->flavour == AsmFlavour::Strict, "");

	EVMDialect const& evmDialect = dynamic_cast<EVMDialect const&>(*_dialect);

	bool optimize = true;
	yul::AsmAnalysisInfo analysisInfo =
		yul::AsmAnalyzer::analyzeStrictAssertCorrect(_dialect, EVMVersion(), _ast);
	NoOutputAssembly assembly;
	CodeTransform transform(assembly, analysisInfo, _ast, evmDialect, optimize);
	try
	{
		transform(_ast);
	}
	catch (StackTooDeepError const&)
	{
		solAssert(!transform.stackErrors().empty(), "Got stack too deep exception that was not stored.");
	}

	std::map<YulString, int> functions;
	for (StackTooDeepError const& error: transform.stackErrors())
		functions[error.functionName] = max(error.depth, functions[error.functionName]);

	return functions;
}
