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

#include <libsolidity/analysis/SyntaxChecker.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ExperimentalFeatures.h>
#include <libsolidity/interface/Version.h>

#include <libyul/optimiser/Semantics.h>
#include <libyul/AST.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/SemVerHandler.h>

#include <libsolutil/UTF8.h>

#include <string>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::util;

bool SyntaxChecker::checkSyntax(ASTNode const& _astRoot)
{
	_astRoot.accept(*this);
	return !Error::containsErrors(m_errorReporter.errors());
}

bool SyntaxChecker::visit(SourceUnit const& _sourceUnit)
{
	m_versionPragmaFound = false;
	m_sourceUnit = &_sourceUnit;
	return true;
}

void SyntaxChecker::endVisit(SourceUnit const& _sourceUnit)
{
	if (!m_versionPragmaFound)
	{
		string errorString("Source file does not specify required compiler version!");
		SemVerVersion recommendedVersion{string(VersionString)};
		if (!recommendedVersion.isPrerelease())
			errorString +=
				" Consider adding \"pragma solidity ^" +
				to_string(recommendedVersion.major()) +
				string(".") +
				to_string(recommendedVersion.minor()) +
				string(".") +
				to_string(recommendedVersion.patch()) +
				string(";\"");

		// when reporting the warning, print the source name only
		m_errorReporter.warning(3420_error, {-1, -1, _sourceUnit.location().sourceName}, errorString);
	}
	if (!m_sourceUnit->annotation().useABICoderV2.set())
		m_sourceUnit->annotation().useABICoderV2 = true;
	m_sourceUnit = nullptr;
}

bool SyntaxChecker::visit(PragmaDirective const& _pragma)
{
	solAssert(!_pragma.tokens().empty(), "");
	solAssert(_pragma.tokens().size() == _pragma.literals().size(), "");
	if (_pragma.tokens()[0] != Token::Identifier)
		m_errorReporter.syntaxError(5226_error, _pragma.location(), "Invalid pragma \"" + _pragma.literals()[0] + "\"");
	else if (_pragma.literals()[0] == "experimental")
	{
		solAssert(m_sourceUnit, "");
		vector<string> literals(_pragma.literals().begin() + 1, _pragma.literals().end());
		if (literals.empty())
			m_errorReporter.syntaxError(
				9679_error,
				_pragma.location(),
				"Experimental feature name is missing."
			);
		else if (literals.size() > 1)
			m_errorReporter.syntaxError(
				6022_error,
				_pragma.location(),
				"Stray arguments."
			);
		else
		{
			string const literal = literals[0];
			if (literal.empty())
				m_errorReporter.syntaxError(3250_error, _pragma.location(), "Empty experimental feature name is invalid.");
			else if (!ExperimentalFeatureNames.count(literal))
				m_errorReporter.syntaxError(8491_error, _pragma.location(), "Unsupported experimental feature name.");
			else if (m_sourceUnit->annotation().experimentalFeatures.count(ExperimentalFeatureNames.at(literal)))
				m_errorReporter.syntaxError(1231_error, _pragma.location(), "Duplicate experimental feature name.");
			else
			{
				auto feature = ExperimentalFeatureNames.at(literal);
				m_sourceUnit->annotation().experimentalFeatures.insert(feature);
				if (!ExperimentalFeatureWithoutWarning.count(feature))
					m_errorReporter.warning(2264_error, _pragma.location(), "Experimental features are turned on. Do not use experimental features on live deployments.");

				if (feature == ExperimentalFeature::ABIEncoderV2)
				{
					if (m_sourceUnit->annotation().useABICoderV2.set())
					{
						if (!*m_sourceUnit->annotation().useABICoderV2)
							m_errorReporter.syntaxError(
								8273_error,
								_pragma.location(),
								"ABI coder v1 has already been selected through \"pragma abicoder v1\"."
							);
					}
					else
						m_sourceUnit->annotation().useABICoderV2 = true;
				}
			}
		}
	}
	else if (_pragma.literals()[0] == "abicoder")
	{
		solAssert(m_sourceUnit, "");
		if (
			_pragma.literals().size() != 2 ||
			!set<string>{"v1", "v2"}.count(_pragma.literals()[1])
		)
			m_errorReporter.syntaxError(
				2745_error,
				_pragma.location(),
				"Expected either \"pragma abicoder v1\" or \"pragma abicoder v2\"."
			);
		else if (m_sourceUnit->annotation().useABICoderV2.set())
			m_errorReporter.syntaxError(
				3845_error,
				_pragma.location(),
				"ABI coder has already been selected for this source unit."
			);
		else
			m_sourceUnit->annotation().useABICoderV2 = (_pragma.literals()[1] == "v2");
	}
	else if (_pragma.literals()[0] == "solidity")
	{
		vector<Token> tokens(_pragma.tokens().begin() + 1, _pragma.tokens().end());
		vector<string> literals(_pragma.literals().begin() + 1, _pragma.literals().end());
		SemVerMatchExpressionParser parser(tokens, literals);
		auto matchExpression = parser.parse();
		// An unparsable version pragma is an unrecoverable fatal error in the parser.
		solAssert(matchExpression.has_value(), "");
		static SemVerVersion const currentVersion{string(VersionString)};
		if (!matchExpression->matches(currentVersion))
			m_errorReporter.syntaxError(
				3997_error,
				_pragma.location(),
				"Source file requires different compiler version (current compiler is " +
				string(VersionString) + ") - note that nightly builds are considered to be "
				"strictly less than the released version"
			);
		m_versionPragmaFound = true;
	}
	else
		m_errorReporter.syntaxError(4936_error, _pragma.location(), "Unknown pragma \"" + _pragma.literals()[0] + "\"");

	return true;
}

bool SyntaxChecker::visit(ModifierDefinition const&)
{
	m_placeholderFound = false;
	return true;
}

void SyntaxChecker::endVisit(ModifierDefinition const& _modifier)
{
	if (_modifier.isImplemented() && !m_placeholderFound)
		m_errorReporter.syntaxError(2883_error, _modifier.body().location(), "Modifier body does not contain '_'.");
	m_placeholderFound = false;
}

void SyntaxChecker::checkSingleStatementVariableDeclaration(ASTNode const& _statement)
{
	auto varDecl = dynamic_cast<VariableDeclarationStatement const*>(&_statement);
	if (varDecl)
		m_errorReporter.syntaxError(9079_error, _statement.location(), "Variable declarations can only be used inside blocks.");
}

bool SyntaxChecker::visit(IfStatement const& _ifStatement)
{
	checkSingleStatementVariableDeclaration(_ifStatement.trueStatement());
	if (Statement const* _statement = _ifStatement.falseStatement())
		checkSingleStatementVariableDeclaration(*_statement);
	return true;
}

bool SyntaxChecker::visit(WhileStatement const& _whileStatement)
{
	m_inLoopDepth++;
	checkSingleStatementVariableDeclaration(_whileStatement.body());
	return true;
}

void SyntaxChecker::endVisit(WhileStatement const&)
{
	m_inLoopDepth--;
}

bool SyntaxChecker::visit(ForStatement const& _forStatement)
{
	m_inLoopDepth++;
	checkSingleStatementVariableDeclaration(_forStatement.body());
	return true;
}

void SyntaxChecker::endVisit(ForStatement const&)
{
	m_inLoopDepth--;
}

bool SyntaxChecker::visit(Block const& _block)
{
	if (_block.unchecked())
	{
		if (m_uncheckedArithmetic)
			m_errorReporter.syntaxError(
				1941_error,
				_block.location(),
				"\"unchecked\" blocks cannot be nested."
			);

		m_uncheckedArithmetic = true;
	}
	return true;
}

void SyntaxChecker::endVisit(Block const& _block)
{
	if (_block.unchecked())
		m_uncheckedArithmetic = false;
}

bool SyntaxChecker::visit(Continue const& _continueStatement)
{
	if (m_inLoopDepth <= 0)
		// we're not in a for/while loop, report syntax error
		m_errorReporter.syntaxError(4123_error, _continueStatement.location(), "\"continue\" has to be in a \"for\" or \"while\" loop.");
	return true;
}

bool SyntaxChecker::visit(Break const& _breakStatement)
{
	if (m_inLoopDepth <= 0)
		// we're not in a for/while loop, report syntax error
		m_errorReporter.syntaxError(6102_error, _breakStatement.location(), "\"break\" has to be in a \"for\" or \"while\" loop.");
	return true;
}

bool SyntaxChecker::visit(Throw const& _throwStatement)
{
	m_errorReporter.syntaxError(
		4538_error,
		_throwStatement.location(),
		"\"throw\" is deprecated in favour of \"revert()\", \"require()\" and \"assert()\"."
	);

	return true;
}

bool SyntaxChecker::visit(Literal const& _literal)
{
	size_t invalidSequence;
	if ((_literal.token() == Token::UnicodeStringLiteral) && !validateUTF8(_literal.value(), invalidSequence))
		m_errorReporter.syntaxError(
			8452_error,
			_literal.location(),
			"Contains invalid UTF-8 sequence at position " + toString(invalidSequence) + "."
		);

	if (_literal.token() != Token::Number)
		return true;

	ASTString const& value = _literal.value();
	solAssert(!value.empty(), "");

	// Generic checks no matter what base this number literal is of:
	if (value.back() == '_')
	{
		m_errorReporter.syntaxError(2090_error, _literal.location(), "Invalid use of underscores in number literal. No trailing underscores allowed.");
		return true;
	}

	if (value.find("__") != ASTString::npos)
	{
		m_errorReporter.syntaxError(2990_error, _literal.location(), "Invalid use of underscores in number literal. Only one consecutive underscores between digits allowed.");
		return true;
	}

	if (!_literal.isHexNumber()) // decimal literal
	{
		if (value.find("._") != ASTString::npos)
			m_errorReporter.syntaxError(3891_error, _literal.location(), "Invalid use of underscores in number literal. No underscores in front of the fraction part allowed.");

		if (value.find("_.") != ASTString::npos)
			m_errorReporter.syntaxError(1023_error, _literal.location(), "Invalid use of underscores in number literal. No underscores in front of the fraction part allowed.");

		if (value.find("_e") != ASTString::npos)
			m_errorReporter.syntaxError(6415_error, _literal.location(), "Invalid use of underscores in number literal. No underscore at the end of the mantissa allowed.");

		if (value.find("e_") != ASTString::npos)
			m_errorReporter.syntaxError(6165_error, _literal.location(), "Invalid use of underscores in number literal. No underscore in front of exponent allowed.");
	}

	return true;
}

bool SyntaxChecker::visit(UnaryOperation const& _operation)
{
	if (_operation.getOperator() == Token::Add)
		m_errorReporter.syntaxError(9636_error, _operation.location(), "Use of unary + is disallowed.");

	return true;
}

bool SyntaxChecker::visit(InlineAssembly const& _inlineAssembly)
{
	if (_inlineAssembly.flags())
		for (auto flag: *_inlineAssembly.flags())
		{
			if (*flag == "memory-safe")
			{
				if (_inlineAssembly.annotation().markedMemorySafe)
					m_errorReporter.syntaxError(
						7026_error,
						_inlineAssembly.location(),
						"Inline assembly marked memory-safe multiple times."
					);
				_inlineAssembly.annotation().markedMemorySafe = true;
			}
			else
				m_errorReporter.warning(
					4430_error,
					_inlineAssembly.location(),
					"Unknown inline assembly flag: \"" + *flag + "\""
				);
		}

	if (!m_useYulOptimizer)
		return false;

	if (yul::MSizeFinder::containsMSize(_inlineAssembly.dialect(), _inlineAssembly.operations()))
		m_errorReporter.syntaxError(
			6553_error,
			_inlineAssembly.location(),
			"The msize instruction cannot be used when the Yul optimizer is activated because "
			"it can change its semantics. Either disable the Yul optimizer or do not use the instruction."
		);

	return false;
}

bool SyntaxChecker::visit(PlaceholderStatement const& _placeholder)
{
	if (m_uncheckedArithmetic)
		m_errorReporter.syntaxError(
			2573_error,
			_placeholder.location(),
			"The placeholder statement \"_\" cannot be used inside an \"unchecked\" block."
		);

	m_placeholderFound = true;
	return true;
}

bool SyntaxChecker::visit(ContractDefinition const& _contract)
{
	m_currentContractKind = _contract.contractKind();

	ASTString const& contractName = _contract.name();
	for (FunctionDefinition const* function: _contract.definedFunctions())
		if (function->name() == contractName)
			m_errorReporter.syntaxError(
				5796_error,
				function->location(),
				"Functions are not allowed to have the same name as the contract. "
				"If you intend this to be a constructor, use \"constructor(...) { ... }\" to define it."
			);
	return true;
}

void SyntaxChecker::endVisit(ContractDefinition const&)
{
	m_currentContractKind = std::nullopt;
}

bool SyntaxChecker::visit(UsingForDirective const& _usingFor)
{
	if (!_usingFor.usesBraces())
		solAssert(
			_usingFor.functionsAndOperators().size() == 1 &&
			!std::get<1>(_usingFor.functionsAndOperators().front())
		);

	if (!m_currentContractKind && !_usingFor.typeName())
		m_errorReporter.syntaxError(
			8118_error,
			_usingFor.location(),
			"The type has to be specified explicitly at file level (cannot use '*')."
		);
	else if (_usingFor.usesBraces() && !_usingFor.typeName())
		m_errorReporter.syntaxError(
			3349_error,
			_usingFor.location(),
			"The type has to be specified explicitly when attaching specific functions."
		);
	if (_usingFor.global() && !_usingFor.typeName())
		m_errorReporter.syntaxError(
			2854_error,
			_usingFor.location(),
			"Can only globally bind functions to specific types."
		);
	if (_usingFor.global() && m_currentContractKind)
		m_errorReporter.syntaxError(
			3367_error,
			_usingFor.location(),
			"\"global\" can only be used at file level."
		);
	if (m_currentContractKind == ContractKind::Interface)
		m_errorReporter.syntaxError(
			9088_error,
			_usingFor.location(),
			"The \"using for\" directive is not allowed inside interfaces."
		);

	return true;
}

bool SyntaxChecker::visit(FunctionDefinition const& _function)
{
	solAssert(_function.isFree() == (m_currentContractKind == std::nullopt), "");

	if (!_function.isFree() && !_function.isConstructor() && _function.noVisibilitySpecified())
	{
		string suggestedVisibility =
			_function.isFallback() ||
			_function.isReceive() ||
			m_currentContractKind == ContractKind::Interface
		? "external" : "public";
		m_errorReporter.syntaxError(
			4937_error,
			_function.location(),
			"No visibility specified. Did you intend to add \"" + suggestedVisibility + "\"?"
		);
	}
	else if (_function.isFree())
	{
		if (!_function.noVisibilitySpecified())
			m_errorReporter.syntaxError(
				4126_error,
				_function.location(),
				"Free functions cannot have visibility."
			);
		if (!_function.isImplemented())
			m_errorReporter.typeError(4668_error, _function.location(), "Free functions must be implemented.");
	}

	if (m_currentContractKind == ContractKind::Interface && !_function.modifiers().empty())
		m_errorReporter.syntaxError(5842_error, _function.location(), "Functions in interfaces cannot have modifiers.");
	else if (!_function.isImplemented() && !_function.modifiers().empty())
		m_errorReporter.syntaxError(2668_error, _function.location(), "Functions without implementation cannot have modifiers.");

	return true;
}

bool SyntaxChecker::visit(FunctionTypeName const& _node)
{
	for (auto const& decl: _node.parameterTypeList()->parameters())
		if (!decl->name().empty())
			m_errorReporter.warning(6162_error, decl->location(), "Naming function type parameters is deprecated.");

	for (auto const& decl: _node.returnParameterTypeList()->parameters())
		if (!decl->name().empty())
			m_errorReporter.syntaxError(7304_error, decl->location(), "Return parameters in function types may not be named.");

	return true;
}

bool SyntaxChecker::visit(StructDefinition const& _struct)
{
	if (_struct.members().empty())
		m_errorReporter.syntaxError(5306_error, _struct.location(), "Defining empty structs is disallowed.");

	return true;
}
