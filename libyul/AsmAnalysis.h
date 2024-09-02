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
 * Analysis part of inline assembly.
 */

#pragma once

#include <liblangutil/Exceptions.h>
#include <liblangutil/EVMVersion.h>

#include <libyul/ASTForward.h>
#include <libyul/Dialect.h>
#include <libyul/Scope.h>

#include <libyul/backends/evm/AbstractAssembly.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <utility>

namespace solidity::langutil
{
class ErrorReporter;
struct SourceLocation;
}

namespace solidity::yul
{

struct AsmAnalysisInfo;

/**
 * Performs the full analysis stage, calls the ScopeFiller internally, then resolves
 * references and performs other checks.
 * If all these checks pass, code generation should not throw errors.
 */
class AsmAnalyzer
{
public:
	explicit AsmAnalyzer(
		AsmAnalysisInfo& _analysisInfo,
		langutil::ErrorReporter& _errorReporter,
		Dialect const& _dialect,
		ExternalIdentifierAccess::Resolver _resolver = ExternalIdentifierAccess::Resolver(),
		std::set<std::string> _dataNames = {}
	):
		m_resolver(std::move(_resolver)),
		m_info(_analysisInfo),
		m_errorReporter(_errorReporter),
		m_dialect(_dialect),
		m_dataNames(std::move(_dataNames))
	{
		if (EVMDialect const* evmDialect = dynamic_cast<EVMDialect const*>(&m_dialect))
			m_evmVersion = evmDialect->evmVersion();
	}

	bool analyze(Block const& _block);

	/// Performs analysis on the outermost code of the given object and returns the analysis info.
	/// Asserts on failure.
	static AsmAnalysisInfo analyzeStrictAssertCorrect(Dialect const& _dialect, Object const& _object);
	static AsmAnalysisInfo analyzeStrictAssertCorrect(
		Dialect const& _dialect,
		Block const& _astRoot,
		std::set<std::string> const& _qualifiedDataNames
	);

	size_t operator()(Literal const& _literal);
	size_t operator()(Identifier const&);
	void operator()(ExpressionStatement const&);
	void operator()(Assignment const& _assignment);
	void operator()(VariableDeclaration const& _variableDeclaration);
	void operator()(FunctionDefinition const& _functionDefinition);
	size_t operator()(FunctionCall const& _functionCall);
	void operator()(If const& _if);
	void operator()(Switch const& _switch);
	void operator()(ForLoop const& _forLoop);
	void operator()(Break const&) { }
	void operator()(Continue const&) { }
	void operator()(Leave const&) { }
	void operator()(Block const& _block);

	/// @returns the worst side effects encountered during analysis (including within defined functions).
	SideEffects const& sideEffects() const { return m_sideEffects; }
private:
	/// Visits the expression, expects that it evaluates to exactly one value.
	/// Reports errors otherwise.
	void expectExpression(Expression const& _expr);
	void expectUnlimitedStringLiteral(Literal const& _literal);

	/// Verifies that a variable to be assigned to exists and can be assigned to.
	void checkAssignment(Identifier const& _variable);

	Scope& scope(Block const* _block);
	void expectValidIdentifier(YulName _identifier, langutil::SourceLocation const& _location);

	bool validateInstructions(evmasm::Instruction _instr, langutil::SourceLocation const& _location);
	bool validateInstructions(std::string const& _instrIdentifier, langutil::SourceLocation const& _location);
	bool validateInstructions(FunctionCall const& _functionCall);

	yul::ExternalIdentifierAccess::Resolver m_resolver;
	Scope* m_currentScope = nullptr;
	/// Variables that are active at the current point in assembly (as opposed to
	/// "part of the scope but not yet declared")
	std::set<Scope::Variable const*> m_activeVariables;
	AsmAnalysisInfo& m_info;
	langutil::ErrorReporter& m_errorReporter;
	langutil::EVMVersion m_evmVersion;
	Dialect const& m_dialect;
	/// Names of data objects to be referenced by builtin functions with literal arguments.
	std::set<std::string> m_dataNames;
	ForLoop const* m_currentForLoop = nullptr;
	/// Worst side effects encountered during analysis (including within defined functions).
	SideEffects m_sideEffects;
};

}
