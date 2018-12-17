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

#include <libsolidity/analysis/SyntaxChecker.h>

#include <libsolidity/analysis/SemVerHandler.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ExperimentalFeatures.h>
#include <libsolidity/interface/Version.h>

#include <liblangutil/ErrorReporter.h>

#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/algorithm/string.hpp>

#include <memory>
#include <string>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::solidity;


bool SyntaxChecker::checkSyntax(ASTNode const& _astRoot)
{
	_astRoot.accept(*this);
	return Error::containsOnlyWarnings(m_errorReporter.errors());
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

		m_errorReporter.warning(_sourceUnit.location(), errorString);
	}
	m_sourceUnit = nullptr;
}

bool SyntaxChecker::visit(PragmaDirective const& _pragma)
{
	solAssert(!_pragma.tokens().empty(), "");
	solAssert(_pragma.tokens().size() == _pragma.literals().size(), "");
	if (_pragma.tokens()[0] != Token::Identifier)
		m_errorReporter.syntaxError(_pragma.location(), "Invalid pragma \"" + _pragma.literals()[0] + "\"");
	else if (_pragma.literals()[0] == "experimental")
	{
		solAssert(m_sourceUnit, "");
		vector<string> literals(_pragma.literals().begin() + 1, _pragma.literals().end());
		if (literals.empty())
			m_errorReporter.syntaxError(
				_pragma.location(),
				"Experimental feature name is missing."
			);
		else if (literals.size() > 1)
			m_errorReporter.syntaxError(
				_pragma.location(),
				"Stray arguments."
			);
		else
		{
			string const literal = literals[0];
			if (literal.empty())
				m_errorReporter.syntaxError(_pragma.location(), "Empty experimental feature name is invalid.");
			else if (!ExperimentalFeatureNames.count(literal))
				m_errorReporter.syntaxError(_pragma.location(), "Unsupported experimental feature name.");
			else if (m_sourceUnit->annotation().experimentalFeatures.count(ExperimentalFeatureNames.at(literal)))
				m_errorReporter.syntaxError(_pragma.location(), "Duplicate experimental feature name.");
			else
			{
				auto feature = ExperimentalFeatureNames.at(literal);
				m_sourceUnit->annotation().experimentalFeatures.insert(feature);
				if (!ExperimentalFeatureOnlyAnalysis.count(feature))
					m_errorReporter.warning(_pragma.location(), "Experimental features are turned on. Do not use experimental features on live deployments.");
			}
		}
	}
	else if (_pragma.literals()[0] == "solidity")
	{
		vector<Token> tokens(_pragma.tokens().begin() + 1, _pragma.tokens().end());
		vector<string> literals(_pragma.literals().begin() + 1, _pragma.literals().end());
		SemVerMatchExpressionParser parser(tokens, literals);
		auto matchExpression = parser.parse();
		SemVerVersion currentVersion{string(VersionString)};
		if (!matchExpression.matches(currentVersion))
			m_errorReporter.syntaxError(
				_pragma.location(),
				"Source file requires different compiler version (current compiler is " +
				string(VersionString) + " - note that nightly builds are considered to be "
				"strictly less than the released version"
			);
		m_versionPragmaFound = true;
	}
	else
		m_errorReporter.syntaxError(_pragma.location(), "Unknown pragma \"" + _pragma.literals()[0] + "\"");
	return true;
}

bool SyntaxChecker::visit(ModifierDefinition const&)
{
	m_placeholderFound = false;
	return true;
}

void SyntaxChecker::endVisit(ModifierDefinition const& _modifier)
{
	if (!m_placeholderFound)
		m_errorReporter.syntaxError(_modifier.body().location(), "Modifier body does not contain '_'.");
	m_placeholderFound = false;
}

void SyntaxChecker::checkSingleStatementVariableDeclaration(ASTNode const& _statement)
{
	auto varDecl = dynamic_cast<VariableDeclarationStatement const*>(&_statement);
	if (varDecl)
		m_errorReporter.syntaxError(_statement.location(), "Variable declarations can only be used inside blocks.");
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

bool SyntaxChecker::visit(Continue const& _continueStatement)
{
	if (m_inLoopDepth <= 0)
		// we're not in a for/while loop, report syntax error
		m_errorReporter.syntaxError(_continueStatement.location(), "\"continue\" has to be in a \"for\" or \"while\" loop.");
	return true;
}

bool SyntaxChecker::visit(Break const& _breakStatement)
{
	if (m_inLoopDepth <= 0)
		// we're not in a for/while loop, report syntax error
		m_errorReporter.syntaxError(_breakStatement.location(), "\"break\" has to be in a \"for\" or \"while\" loop.");
	return true;
}

bool SyntaxChecker::visit(Throw const& _throwStatement)
{
	m_errorReporter.syntaxError(
		_throwStatement.location(),
		"\"throw\" is deprecated in favour of \"revert()\", \"require()\" and \"assert()\"."
	);

	return true;
}

bool SyntaxChecker::visit(Literal const& _literal)
{
	if (_literal.token() != Token::Number)
		return true;

	ASTString const& value = _literal.value();
	solAssert(!value.empty(), "");

	// Generic checks no matter what base this number literal is of:
	if (value.back() == '_')
	{
		m_errorReporter.syntaxError(_literal.location(), "Invalid use of underscores in number literal. No trailing underscores allowed.");
		return true;
	}

	if (value.find("__") != ASTString::npos)
	{
		m_errorReporter.syntaxError(_literal.location(), "Invalid use of underscores in number literal. Only one consecutive underscores between digits allowed.");
		return true;
	}

	if (!_literal.isHexNumber()) // decimal literal
	{
		if (value.find("._") != ASTString::npos)
			m_errorReporter.syntaxError(_literal.location(), "Invalid use of underscores in number literal. No underscores in front of the fraction part allowed.");

		if (value.find("_.") != ASTString::npos)
			m_errorReporter.syntaxError(_literal.location(), "Invalid use of underscores in number literal. No underscores in front of the fraction part allowed.");

		if (value.find("_e") != ASTString::npos)
			m_errorReporter.syntaxError(_literal.location(), "Invalid use of underscores in number literal. No underscore at the end of the mantissa allowed.");

		if (value.find("e_") != ASTString::npos)
			m_errorReporter.syntaxError(_literal.location(), "Invalid use of underscores in number literal. No underscore in front of exponent allowed.");
	}

	return true;
}

bool SyntaxChecker::visit(UnaryOperation const& _operation)
{
	if (_operation.getOperator() == Token::Add)
		m_errorReporter.syntaxError(_operation.location(), "Use of unary + is disallowed.");

	return true;
}

bool SyntaxChecker::visit(PlaceholderStatement const&)
{
	m_placeholderFound = true;
	return true;
}

bool SyntaxChecker::visit(ContractDefinition const& _contract)
{
	m_isInterface = _contract.contractKind() == ContractDefinition::ContractKind::Interface;

	ASTString const& contractName = _contract.name();
	for (FunctionDefinition const* function: _contract.definedFunctions())
		if (function->name() == contractName)
			m_errorReporter.syntaxError(function->location(),
				"Functions are not allowed to have the same name as the contract. "
				"If you intend this to be a constructor, use \"constructor(...) { ... }\" to define it."
			);
	return true;
}

bool SyntaxChecker::visit(FunctionDefinition const& _function)
{
	if (_function.noVisibilitySpecified())
	{
		string suggestedVisibility = _function.isFallback() || m_isInterface ? "external" : "public";
		m_errorReporter.syntaxError(
			_function.location(),
			"No visibility specified. Did you intend to add \"" + suggestedVisibility + "\"?"
		);
	}

	if (!_function.isImplemented() && !_function.modifiers().empty())
		m_errorReporter.syntaxError(_function.location(), "Functions without implementation cannot have modifiers.");

	return true;
}

bool SyntaxChecker::visit(FunctionTypeName const& _node)
{
	for (auto const& decl: _node.parameterTypeList()->parameters())
		if (!decl->name().empty())
			m_errorReporter.warning(decl->location(), "Naming function type parameters is deprecated.");

	for (auto const& decl: _node.returnParameterTypeList()->parameters())
		if (!decl->name().empty())
			m_errorReporter.syntaxError(decl->location(), "Return parameters in function types may not be named.");

	return true;
}

bool SyntaxChecker::visit(VariableDeclarationStatement const& _statement)
{
	// Report if none of the variable components in the tuple have a name (only possible via deprecated "var")
	if (boost::algorithm::all_of_equal(_statement.declarations(), nullptr))
		m_errorReporter.syntaxError(
			_statement.location(),
			"The use of the \"var\" keyword is disallowed. The declaration part of the statement can be removed, since it is empty."
		);

	return true;
}

bool SyntaxChecker::visit(StructDefinition const& _struct)
{
	if (_struct.members().empty())
		m_errorReporter.syntaxError(_struct.location(), "Defining empty structs is disallowed.");

	return true;
}
