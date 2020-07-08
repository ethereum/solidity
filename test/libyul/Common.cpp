// SPDX-License-Identifier: GPL-3.0
/**
 * @date 2017
 * Common functions the Yul tests.
 */

#include <test/libyul/Common.h>

#include <test/Common.h>

#include <liblangutil/SourceReferenceFormatter.h>

#include <libyul/optimiser/Disambiguator.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AssemblyStack.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/wasm/WasmDialect.h>

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
	return _yul ? yul::Dialect::yulDeprecated() : yul::EVMDialect::strictAssemblyForEVM(solidity::test::CommonOptions::get().evmVersion());
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
		solidity::test::CommonOptions::get().evmVersion(),
		_yul ? AssemblyStack::Language::Yul : AssemblyStack::Language::StrictAssembly,
		solidity::test::CommonOptions::get().optimize ?
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
	if (!parserResult->code || errorReporter.hasErrors())
		return {};
	shared_ptr<AsmAnalysisInfo> analysisInfo = make_shared<AsmAnalysisInfo>();
	AsmAnalyzer analyzer(*analysisInfo, errorReporter, _dialect, {}, parserResult->dataNames());
	// TODO this should be done recursively.
	if (!analyzer.analyze(*parserResult->code) || errorReporter.hasErrors())
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

namespace
{
std::map<string const, yul::Dialect const& (*)(langutil::EVMVersion)> const validDialects = {
	{
		"evm",
		[](langutil::EVMVersion _evmVersion) -> yul::Dialect const&
		{ return yul::EVMDialect::strictAssemblyForEVMObjects(_evmVersion); }
	},
	{
		"evmTyped",
		[](langutil::EVMVersion _evmVersion) -> yul::Dialect const&
		{ return yul::EVMDialectTyped::instance(_evmVersion); }
	},
	{
		"yul",
		[](langutil::EVMVersion) -> yul::Dialect const&
		{ return yul::Dialect::yulDeprecated(); }
	},
	{
		"ewasm",
		[](langutil::EVMVersion) -> yul::Dialect const&
		{ return yul::WasmDialect::instance(); }
	}
};

vector<string> validDialectNames()
{
	vector<string> names{size(validDialects), ""};
	transform(begin(validDialects), end(validDialects), names.begin(), [](auto const& dialect) { return dialect.first; });

	return names;
}
}

yul::Dialect const& yul::test::dialect(std::string const& _name, langutil::EVMVersion _evmVersion)
{
	if (!validDialects.count(_name))
		BOOST_THROW_EXCEPTION(runtime_error{
			"Invalid Dialect \"" +
			_name +
			"\". Valid dialects are " +
			util::joinHumanReadable(validDialectNames(), ", ", " and ") +
			"."
		});

	return validDialects.at(_name)(_evmVersion);
}
