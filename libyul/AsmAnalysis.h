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
 * Analysis part of inline assembly.
 */

#pragma once

#include <liblangutil/Exceptions.h>
#include <liblangutil/EVMVersion.h>

#include <libyul/Dialect.h>
#include <libyul/AsmScope.h>
#include <libyul/AsmDataForward.h>

#include <libyul/backends/evm/AbstractAssembly.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <functional>
#include <list>
#include <memory>

namespace langutil
{
class ErrorReporter;
struct SourceLocation;
}

namespace yul
{

struct AsmAnalysisInfo;

/**
 * Performs the full analysis stage, calls the ScopeFiller internally, then resolves
 * references and performs other checks.
 * If all these checks pass, code generation should not throw errors.
 */
class AsmAnalyzer: public boost::static_visitor<bool>
{
public:
	explicit AsmAnalyzer(
		AsmAnalysisInfo& _analysisInfo,
		langutil::ErrorReporter& _errorReporter,
		boost::optional<langutil::Error::Type> _errorTypeForLoose,
		std::shared_ptr<Dialect const> _dialect,
		ExternalIdentifierAccess::Resolver const& _resolver = ExternalIdentifierAccess::Resolver()
	):
		m_resolver(_resolver),
		m_info(_analysisInfo),
		m_errorReporter(_errorReporter),
		m_dialect(std::move(_dialect)),
		m_errorTypeForLoose(_errorTypeForLoose)
	{
		if (EVMDialect const* evmDialect = dynamic_cast<EVMDialect const*>(m_dialect.get()))
			m_evmVersion = evmDialect->evmVersion();
	}

	bool analyze(Block const& _block);

	static AsmAnalysisInfo analyzeStrictAssertCorrect(
		std::shared_ptr<Dialect const> _dialect,
		Block const& _ast
	);

	bool operator()(Instruction const&);
	bool operator()(Literal const& _literal);
	bool operator()(Identifier const&);
	bool operator()(FunctionalInstruction const& _functionalInstruction);
	bool operator()(Label const& _label);
	bool operator()(ExpressionStatement const&);
	bool operator()(StackAssignment const&);
	bool operator()(Assignment const& _assignment);
	bool operator()(VariableDeclaration const& _variableDeclaration);
	bool operator()(FunctionDefinition const& _functionDefinition);
	bool operator()(FunctionCall const& _functionCall);
	bool operator()(If const& _if);
	bool operator()(Switch const& _switch);
	bool operator()(ForLoop const& _forLoop);
	bool operator()(Break const&);
	bool operator()(Continue const&);
	bool operator()(Block const& _block);

private:
	/// Visits the statement and expects it to deposit one item onto the stack.
	bool expectExpression(Expression const& _expr);
	bool expectDeposit(int _deposit, int _oldHeight, langutil::SourceLocation const& _location);

	/// Verifies that a variable to be assigned to exists and has the same size
	/// as the value, @a _valueSize, unless that is equal to -1.
	bool checkAssignment(Identifier const& _assignment, size_t _valueSize = size_t(-1));

	Scope& scope(Block const* _block);
	void expectValidType(std::string const& type, langutil::SourceLocation const& _location);
	void warnOnInstructions(dev::eth::Instruction _instr, langutil::SourceLocation const& _location);

	/// Depending on @a m_flavour and @a m_errorTypeForLoose, throws an internal compiler
	/// exception (if the flavour is not Loose), reports an error/warning
	/// (if m_errorTypeForLoose is set) or does nothing.
	void checkLooseFeature(langutil::SourceLocation const& _location, std::string const& _description);

	int m_stackHeight = 0;
	yul::ExternalIdentifierAccess::Resolver m_resolver;
	Scope* m_currentScope = nullptr;
	/// Variables that are active at the current point in assembly (as opposed to
	/// "part of the scope but not yet declared")
	std::set<Scope::Variable const*> m_activeVariables;
	AsmAnalysisInfo& m_info;
	langutil::ErrorReporter& m_errorReporter;
	langutil::EVMVersion m_evmVersion;
	std::shared_ptr<Dialect const> m_dialect;
	boost::optional<langutil::Error::Type> m_errorTypeForLoose;
	ForLoop const* m_currentForLoop = nullptr;
};

}
