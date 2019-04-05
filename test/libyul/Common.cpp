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

using namespace std;
using namespace langutil;
using namespace yul;

namespace
{
shared_ptr<Dialect> defaultDialect(bool _yul)
{
	return _yul ? yul::Dialect::yul() : yul::EVMDialect::strictAssemblyForEVM(dev::test::Options::get().evmVersion());
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
		dev::test::Options::get().evmVersion(),
		_yul ? AssemblyStack::Language::Yul : AssemblyStack::Language::StrictAssembly,
		dev::test::Options::get().optimize ?
			dev::solidity::OptimiserSettings::standard() :
			dev::solidity::OptimiserSettings::minimal()
	);
	if (!stack.parseAndAnalyze("", _source) || !stack.errors().empty())
		BOOST_FAIL("Invalid source.");
	return make_pair(stack.parserResult()->code, stack.parserResult()->analysisInfo);
}

yul::Block yul::test::disambiguate(string const& _source, bool _yul)
{
	auto result = parse(_source, _yul);
	return boost::get<Block>(Disambiguator(*defaultDialect(_yul), *result.second, {})(*result.first));
}

string yul::test::format(string const& _source, bool _yul)
{
	return yul::AsmPrinter(_yul)(*parse(_source, _yul).first);
}
