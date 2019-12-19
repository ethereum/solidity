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
 * @date 2017
 * Common functions the Yul tests.
 */

#include <test/libyul/Common.h>

#include <test/Options.h>

#include <liblangutil/SourceReferenceFormatter.h>

#include <libyul/optimiser/Disambiguator.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AssemblyStack.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <liblangutil/Scanner.h>
#include <liblangutil/ErrorReporter.h>

#include <boost/test/unit_test.hpp>

#include <variant>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::langutil;

namespace
{
Dialect const& defaultDialect(bool _yul)
{
	return _yul ? yul::Dialect::yul() : yul::EVMDialect::strictAssemblyForEVM(solidity::test::Options::get().evmVersion());
}
}

void yul::test::printErrors(ErrorList const& _errors)
{
	SourceReferenceFormatter formatter(cout);

	for (auto const& error: _errors)
		formatter.printErrorInformation(*error);
}


pair<shared_ptr<Block>, shared_ptr<yul::AsmAnalysisInfo>> yul::test::parse(string const& _source, bool _yul)
{
	AssemblyStack stack(
		solidity::test::Options::get().evmVersion(),
		_yul ? AssemblyStack::Language::Yul : AssemblyStack::Language::StrictAssembly,
		solidity::test::Options::get().optimize ?
			solidity::frontend::OptimiserSettings::standard() :
			solidity::frontend::OptimiserSettings::minimal()
	);
	if (!stack.parseAndAnalyze("", _source) || !stack.errors().empty())
		BOOST_FAIL("Invalid source.");
	return make_pair(stack.parserResult()->code, stack.parserResult()->analysisInfo);
}

pair<shared_ptr<Block>, shared_ptr<yul::AsmAnalysisInfo>> yul::test::parse(
	string const& _source,
	Dialect const& _dialect,
	ErrorList& _errors
)
{
	ErrorReporter errorReporter(_errors);
	shared_ptr<Scanner> scanner = make_shared<Scanner>(CharStream(_source, ""));
	shared_ptr<Object> parserResult = yul::ObjectParser(errorReporter, _dialect).parse(scanner, false);
	if (!parserResult)
		return {};
	if (!parserResult->code || !errorReporter.errors().empty())
		return {};
	shared_ptr<AsmAnalysisInfo> analysisInfo = make_shared<AsmAnalysisInfo>();
	AsmAnalyzer analyzer(*analysisInfo, errorReporter, _dialect, {}, parserResult->dataNames());
	// TODO this should be done recursively.
	if (!analyzer.analyze(*parserResult->code) || !errorReporter.errors().empty())
		return {};
	return {std::move(parserResult->code), std::move(analysisInfo)};
}

yul::Block yul::test::disambiguate(string const& _source, bool _yul)
{
	auto result = parse(_source, _yul);
	return std::get<Block>(Disambiguator(defaultDialect(_yul), *result.second, {})(*result.first));
}

string yul::test::format(string const& _source, bool _yul)
{
	return yul::AsmPrinter()(*parse(_source, _yul).first);
}
