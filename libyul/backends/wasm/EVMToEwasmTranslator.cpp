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
// SPDX-License-Identifier: GPL-3.0
/**
 * Translates Yul code from EVM dialect to Ewasm dialect.
 */

#include <libyul/backends/wasm/EVMToEwasmTranslator.h>

#include <libyul/backends/wasm/WordSizeTransform.h>

#include <libyul/backends/wasm/polyfill/arithmetic.h>
#include <libyul/backends/wasm/polyfill/bitwise.h>
#include <libyul/backends/wasm/polyfill/conversion.h>
#include <libyul/backends/wasm/polyfill/eei.h>
#include <libyul/backends/wasm/polyfill/keccak.h>
#include <libyul/backends/wasm/polyfill/logical.h>
#include <libyul/backends/wasm/polyfill/memory.h>

#include <libyul/backends/wasm/WasmDialect.h>
#include <libyul/optimiser/ExpressionSplitter.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/optimiser/MainFunction.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/Disambiguator.h>
#include <libyul/optimiser/NameDisplacer.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/ForLoopConditionIntoBody.h>

#include <libyul/AsmParser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/Object.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/SourceReferenceFormatter.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::langutil;

namespace
{

string const polyfill{
	"{" + solidity::yul::wasm::polyfill::arithmetic +
	solidity::yul::wasm::polyfill::bitwise +
	solidity::yul::wasm::polyfill::conversion +
	solidity::yul::wasm::polyfill::eei +
	solidity::yul::wasm::polyfill::keccak +
	solidity::yul::wasm::polyfill::logical +
	solidity::yul::wasm::polyfill::memory + "}"
};

}

Object EVMToEwasmTranslator::run(Object const& _object)
{
	if (!m_polyfill)
		parsePolyfill();

	Block ast = std::get<Block>(Disambiguator(m_dialect, *_object.analysisInfo)(*_object.code));
	set<YulString> reservedIdentifiers;
	NameDispenser nameDispenser{m_dialect, ast, reservedIdentifiers};
	OptimiserStepContext context{m_dialect, nameDispenser, reservedIdentifiers};

	FunctionHoister::run(context, ast);
	FunctionGrouper::run(context, ast);
	MainFunction::run(context, ast);
	ForLoopConditionIntoBody::run(context, ast);
	ExpressionSplitter::run(context, ast);
	WordSizeTransform::run(m_dialect, WasmDialect::instance(), ast, nameDispenser);

	NameDisplacer{nameDispenser, m_polyfillFunctions}(ast);
	for (auto const& st: m_polyfill->statements)
		ast.statements.emplace_back(ASTCopier{}.translate(st));

	Object ret;
	ret.name = _object.name;
	ret.code = make_shared<Block>(move(ast));
	ret.analysisInfo = make_shared<AsmAnalysisInfo>();

	ErrorList errors;
	ErrorReporter errorReporter(errors);
	AsmAnalyzer analyzer(*ret.analysisInfo, errorReporter, WasmDialect::instance(), {}, _object.qualifiedDataNames());
	if (!analyzer.analyze(*ret.code))
	{
		string message = "Invalid code generated after EVM to wasm translation.\n";
		message += "Note that the source locations in the errors below will reference the original, not the translated code.\n";
		message += "Translated code:\n";
		message += "----------------------------------\n";
		message += ret.toString(&WasmDialect::instance());
		message += "----------------------------------\n";
		for (auto const& err: errors)
			message += langutil::SourceReferenceFormatter::formatErrorInformation(*err);
		yulAssert(false, message);
	}

	for (auto const& subObjectNode: _object.subObjects)
		if (Object const* subObject = dynamic_cast<Object const*>(subObjectNode.get()))
			ret.subObjects.push_back(make_shared<Object>(run(*subObject)));
		else
			ret.subObjects.push_back(make_shared<Data>(dynamic_cast<Data const&>(*subObjectNode)));
	ret.subIndexByName = _object.subIndexByName;

	return ret;
}

void EVMToEwasmTranslator::parsePolyfill()
{
	ErrorList errors;
	ErrorReporter errorReporter(errors);
	shared_ptr<Scanner> scanner{make_shared<Scanner>(CharStream(polyfill, ""))};
	m_polyfill = Parser(errorReporter, WasmDialect::instance()).parse(scanner, false);
	if (!errors.empty())
	{
		string message;
		for (auto const& err: errors)
			message += langutil::SourceReferenceFormatter::formatErrorInformation(*err);
		yulAssert(false, message);
	}

	m_polyfillFunctions.clear();
	for (auto const& statement: m_polyfill->statements)
		m_polyfillFunctions.insert(std::get<FunctionDefinition>(statement).name);
}
