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

#include <test/Common.h>

#include <libyul/optimiser/Disambiguator.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmPrinter.h>
#include <libyul/YulStack.h>
#include <libyul/AST.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <liblangutil/DebugInfoSelection.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Scanner.h>

#include <boost/test/unit_test.hpp>

#include <variant>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::langutil;

namespace
{
Dialect const& defaultDialect()
{
	return yul::EVMDialect::strictAssemblyForEVM(solidity::test::CommonOptions::get().evmVersion());
}
}

std::pair<std::shared_ptr<AST const>, std::shared_ptr<yul::AsmAnalysisInfo>> yul::test::parse(std::string const& _source)
{
	YulStack stack(
		solidity::test::CommonOptions::get().evmVersion(),
		solidity::test::CommonOptions::get().eofVersion(),
		YulStack::Language::StrictAssembly,
		solidity::test::CommonOptions::get().optimize ?
			solidity::frontend::OptimiserSettings::standard() :
			solidity::frontend::OptimiserSettings::minimal(),
		DebugInfoSelection::ExceptExperimental()
	);
	if (!stack.parseAndAnalyze("", _source) || !stack.errors().empty())
		BOOST_FAIL("Invalid source.");
	return std::make_pair(stack.parserResult()->code(), stack.parserResult()->analysisInfo);
}

std::pair<std::shared_ptr<Object>, std::shared_ptr<yul::AsmAnalysisInfo>> yul::test::parse(
	std::string const& _source,
	Dialect const& _dialect,
	ErrorList& _errors
)
{
	ErrorReporter errorReporter(_errors);
	CharStream stream(_source, "");
	std::shared_ptr<Scanner> scanner = std::make_shared<Scanner>(stream);
	std::shared_ptr<Object> parserResult = yul::ObjectParser(errorReporter, _dialect).parse(scanner, false);
	if (!parserResult)
		return {};
	if (!parserResult->hasCode() || errorReporter.hasErrors())
		return {};
	std::shared_ptr<AsmAnalysisInfo> analysisInfo = std::make_shared<AsmAnalysisInfo>();
	AsmAnalyzer analyzer(*analysisInfo, errorReporter, _dialect, {}, parserResult->qualifiedDataNames());
	// TODO this should be done recursively.
	if (!analyzer.analyze(parserResult->code()->root()) || errorReporter.hasErrors())
		return {};
	return {std::move(parserResult), std::move(analysisInfo)};
}

yul::Block yul::test::disambiguate(std::string const& _source)
{
	auto result = parse(_source);
	return std::get<Block>(Disambiguator(defaultDialect(), *result.second, {})(result.first->root()));
}

std::string yul::test::format(std::string const& _source)
{
	return yul::AsmPrinter()(parse(_source).first->root());
}

namespace
{
std::map<std::string const, yul::Dialect const& (*)(langutil::EVMVersion)> const validDialects = {
	{
		"evm",
		[](langutil::EVMVersion _evmVersion) -> yul::Dialect const&
		{ return yul::EVMDialect::strictAssemblyForEVMObjects(_evmVersion); }
	}
};

	std::vector<std::string> validDialectNames()
{
	std::vector<std::string> names{size(validDialects), ""};
	std::transform(begin(validDialects), end(validDialects), names.begin(), [](auto const& dialect) { return dialect.first; });

	return names;
}
}

yul::Dialect const& yul::test::dialect(std::string const& _name, langutil::EVMVersion _evmVersion)
{
	if (!validDialects.count(_name))
		BOOST_THROW_EXCEPTION(std::runtime_error{
			"Invalid Dialect \"" +
			_name +
			"\". Valid dialects are " +
			util::joinHumanReadable(validDialectNames(), ", ", " and ") +
			"."
		});

	return validDialects.at(_name)(_evmVersion);
}
