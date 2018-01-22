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
 * Optimiser suite that combines all steps and also provides the settings for the heuristics.
 */

#include <libjulia/optimiser/Suite.h>

#include <libjulia/optimiser/Disambiguator.h>
#include <libjulia/optimiser/FunctionGrouper.h>
#include <libjulia/optimiser/FunctionHoister.h>
#include <libjulia/optimiser/ExpressionInliner.h>
#include <libjulia/optimiser/FullInliner.h>
#include <libjulia/optimiser/Rematerialiser.h>
#include <libjulia/optimiser/UnusedPruner.h>
#include <libjulia/optimiser/NameShortener.h>
#include <libjulia/optimiser/ExpressionSimplifier.h>

#include <libjulia/Builtins.h>

#include <libsolidity/inlineasm/AsmAnalysisInfo.h>
#include <libsolidity/inlineasm/AsmData.h>

#include <libsolidity/inlineasm/AsmPrinter.h>

#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace dev::julia;

void OptimiserSuite::run(
	Block& _ast,
	solidity::assembly::AsmAnalysisInfo const& _analysisInfo,
	set<string> const& _externallyUsedIdentifiers
)
{
	solidity::assembly::AsmPrinter p;

	set<string> reservedIdentifiers = _externallyUsedIdentifiers;
	reservedIdentifiers += BuiltinFunctions::names(false);

	cout << "===============================================" << endl;
	cout << "Optimizer input: ----------------------------------------------" << endl;
	cout << p(_ast) << endl;
	Block ast = boost::get<Block>(Disambiguator(_analysisInfo, reservedIdentifiers)(_ast));
	cout << "Disambiguator: ----------------------------------------------" << endl;
	cout << p(ast) << endl;

	NameShortener shortener(ast, reservedIdentifiers, 10);
	ast = boost::get<Block>(shortener(ast));
	cout << "Name shortener: ----------------------------------------------" << endl;
	cout << p(ast) << endl;

	(FunctionHoister{})(ast);
	(FunctionGrouper{})(ast);
	cout << "Function hoister and grouper: ----------------------------------------------" << endl;
	cout << p(ast) << endl;

	for (size_t i = 0; i < 3; i++)
	{
		Rematerialiser{}(ast);
		cout << "Rematerialiser: ----------------------------------------------" << endl;
		cout << p(ast) << endl;
		ExpressionSimplifier{}(ast);
		cout << "Simplifier: ----------------------------------------------" << endl;
		cout << p(ast) << endl;
		ExpressionInliner(ast).run();
		cout << "Functional Inliner: ----------------------------------------------" << endl;
		cout << p(ast) << endl;
		Rematerialiser{}(ast);
		cout << "Rematerialiser: ----------------------------------------------" << endl;
		cout << p(ast) << endl;
		FullInliner(ast).run();
		cout << "Full Inliner: ----------------------------------------------" << endl;
		cout << p(ast) << endl;
		Rematerialiser{}(ast);
		cout << "Rematerialiser: ----------------------------------------------" << endl;
		cout << p(ast) << endl;
		UnusedPruner::runUntilStabilised(ast, reservedIdentifiers);
		cout << "Unused Pruner: ----------------------------------------------" << endl;
		cout << p(ast) << endl;
		cout << "===============================================" << endl;
	}
	cout << "===============================================" << endl;

	_ast = std::move(ast);
}
