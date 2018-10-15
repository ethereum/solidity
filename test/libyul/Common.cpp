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

#include <libyul/optimiser/Disambiguator.h>

#include <libsolidity/parsing/Scanner.h>

#include <libsolidity/inlineasm/AsmParser.h>
#include <libsolidity/inlineasm/AsmAnalysis.h>
#include <libsolidity/inlineasm/AsmPrinter.h>

#include <libsolidity/interface/SourceReferenceFormatter.h>
#include <libsolidity/interface/ErrorReporter.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace dev::yul;
using namespace dev::solidity;

void dev::yul::test::printErrors(ErrorList const& _errors, Scanner const& _scanner)
{
	SourceReferenceFormatter formatter(cout, [&](std::string const&) -> Scanner const& { return _scanner; });

	for (auto const& error: _errors)
		formatter.printExceptionInformation(
			*error,
			(error->type() == Error::Type::Warning) ? "Warning" : "Error"
		);
}


pair<shared_ptr<Block>, shared_ptr<assembly::AsmAnalysisInfo>> dev::yul::test::parse(string const& _source, bool _yul)
{
	auto flavour = _yul ? assembly::AsmFlavour::Yul : assembly::AsmFlavour::Strict;
	ErrorList errors;
	ErrorReporter errorReporter(errors);
	auto scanner = make_shared<Scanner>(CharStream(_source), "");
	auto parserResult = assembly::Parser(errorReporter, flavour).parse(scanner, false);
	if (parserResult)
	{
		BOOST_REQUIRE(errorReporter.errors().empty());
		auto analysisInfo = make_shared<assembly::AsmAnalysisInfo>();
		assembly::AsmAnalyzer analyzer(
			*analysisInfo,
			errorReporter,
			dev::test::Options::get().evmVersion(),
			boost::none,
			flavour
		);
		if (analyzer.analyze(*parserResult))
		{
			BOOST_REQUIRE(errorReporter.errors().empty());
			return make_pair(parserResult, analysisInfo);
		}
	}
	printErrors(errors, *scanner);
	BOOST_FAIL("Invalid source.");

	// Unreachable.
	return {};
}

assembly::Block dev::yul::test::disambiguate(string const& _source, bool _yul)
{
	auto result = parse(_source, _yul);
	return boost::get<Block>(Disambiguator(*result.second)(*result.first));
}

string dev::yul::test::format(string const& _source, bool _yul)
{
	return assembly::AsmPrinter(_yul)(*parse(_source, _yul).first);
}
