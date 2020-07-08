// SPDX-License-Identifier: GPL-3.0
/**
 * Module responsible for registering identifiers inside their scopes.
 */

#pragma once

#include <libyul/AsmDataForward.h>

#include <functional>
#include <memory>

namespace solidity::langutil
{
class ErrorReporter;
struct SourceLocation;
}

namespace solidity::yul
{

struct TypedName;
struct Scope;
struct AsmAnalysisInfo;

/**
 * Fills scopes with identifiers and checks for name clashes.
 * Does not resolve references.
 */
class ScopeFiller
{
public:
	ScopeFiller(AsmAnalysisInfo& _info, langutil::ErrorReporter& _errorReporter);

	bool operator()(Literal const&) { return true; }
	bool operator()(Identifier const&) { return true; }
	bool operator()(ExpressionStatement const& _expr);
	bool operator()(Assignment const&) { return true; }
	bool operator()(VariableDeclaration const& _variableDeclaration);
	bool operator()(FunctionDefinition const& _functionDefinition);
	bool operator()(FunctionCall const&) { return true; }
	bool operator()(If const& _if);
	bool operator()(Switch const& _switch);
	bool operator()(ForLoop const& _forLoop);
	bool operator()(Break const&) { return true; }
	bool operator()(Continue const&) { return true; }
	bool operator()(Leave const&) { return true; }
	bool operator()(Block const& _block);

private:
	bool registerVariable(
		TypedName const& _name,
		langutil::SourceLocation const& _location,
		Scope& _scope
	);
	bool registerFunction(FunctionDefinition const& _funDef);

	Scope& scope(Block const* _block);

	Scope* m_currentScope = nullptr;
	AsmAnalysisInfo& m_info;
	langutil::ErrorReporter& m_errorReporter;
};

}
