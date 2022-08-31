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
#include <libyul/backends/wasm/WasmDialect.h>
#include <libyul/optimiser/ExpressionSplitter.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/optimiser/MainFunction.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/Disambiguator.h>
#include <libyul/optimiser/NameDisplacer.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/ForLoopConditionIntoBody.h>

#include <libyul/AST.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/Object.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/SourceReferenceFormatter.h>
#include <liblangutil/CharStreamProvider.h>

#include <libsolidity/interface/OptimiserSettings.h>

// The following headers are generated from the
// yul files placed in libyul/backends/wasm/polyfill.

#include <ewasmPolyfills/Arithmetic.h>
#include <ewasmPolyfills/Bitwise.h>
#include <ewasmPolyfills/Comparison.h>
#include <ewasmPolyfills/Conversion.h>
#include <ewasmPolyfills/Interface.h>
#include <ewasmPolyfills/Keccak.h>
#include <ewasmPolyfills/Logical.h>
#include <ewasmPolyfills/Memory.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::langutil;

Object EVMToEwasmTranslator::run(Object const& _object)
{
	if (!m_polyfill)
		parsePolyfill();

	Block ast = std::get<Block>(Disambiguator(m_dialect, *_object.analysisInfo)(*_object.code));
	set<YulString> reservedIdentifiers;
	NameDispenser nameDispenser{m_dialect, ast, reservedIdentifiers};
	// expectedExecutionsPerDeployment is currently unused.
	OptimiserStepContext context{
		m_dialect,
		nameDispenser,
		reservedIdentifiers,
		frontend::OptimiserSettings::standard().expectedExecutionsPerDeployment
	};

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
	ret.code = make_shared<Block>(std::move(ast));
	ret.debugData = _object.debugData;
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
			message += langutil::SourceReferenceFormatter::formatErrorInformation(*err, m_charStreamProvider);
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
	CharStream charStream(
		"{" +
			string(solidity::yul::wasm::polyfill::Arithmetic) +
			string(solidity::yul::wasm::polyfill::Bitwise) +
			string(solidity::yul::wasm::polyfill::Comparison) +
			string(solidity::yul::wasm::polyfill::Conversion) +
			string(solidity::yul::wasm::polyfill::Interface) +
			string(solidity::yul::wasm::polyfill::Keccak) +
			string(solidity::yul::wasm::polyfill::Logical) +
			string(solidity::yul::wasm::polyfill::Memory) +
		"}", "");

	// Passing an empty SourceLocation() here is a workaround to prevent a crash
	// when compiling from yul->ewasm. We're stripping nativeLocation and
	// originLocation from the AST (but we only really need to strip nativeLocation)
	m_polyfill = Parser(errorReporter, WasmDialect::instance(), langutil::SourceLocation()).parse(charStream);
	if (!errors.empty())
	{
		string message;
		for (auto const& err: errors)
			message += langutil::SourceReferenceFormatter::formatErrorInformation(
				*err,
				SingletonCharStreamProvider(charStream)
			);
		yulAssert(false, message);
	}

	m_polyfillFunctions.clear();
	for (auto const& statement: m_polyfill->statements)
		m_polyfillFunctions.insert(std::get<FunctionDefinition>(statement).name);
}
