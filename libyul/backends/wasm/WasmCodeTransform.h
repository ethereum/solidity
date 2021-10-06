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
 * Common code generator for translating Yul / inline assembly to Wasm.
 */

#pragma once

#include <libyul/backends/wasm/WasmAST.h>
#include <libyul/ASTForward.h>
#include <libyul/Dialect.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/optimiser/TypeInfo.h>

#include <libsolutil/Common.h>
#include <libsolutil/Numeric.h>

#include <stack>
#include <map>

namespace solidity::yul
{
struct AsmAnalysisInfo;

class WasmCodeTransform
{
public:
	static wasm::Module run(Dialect const& _dialect, yul::Block const& _ast);

public:
	wasm::Expression operator()(yul::Literal const& _literal);
	wasm::Expression operator()(yul::Identifier const& _identifier);
	wasm::Expression operator()(yul::FunctionCall const&);
	wasm::Expression operator()(yul::ExpressionStatement const& _statement);
	wasm::Expression operator()(yul::Assignment const& _assignment);
	wasm::Expression operator()(yul::VariableDeclaration const& _varDecl);
	wasm::Expression operator()(yul::If const& _if);
	wasm::Expression operator()(yul::Switch const& _switch);
	wasm::Expression operator()(yul::FunctionDefinition const&);
	wasm::Expression operator()(yul::ForLoop const&);
	wasm::Expression operator()(yul::Break const&);
	wasm::Expression operator()(yul::Continue const&);
	wasm::Expression operator()(yul::Leave const&);
	wasm::Expression operator()(yul::Block const& _block);

private:
	WasmCodeTransform(
		Dialect const& _dialect,
		Block const& _ast,
		TypeInfo& _typeInfo
	):
		m_dialect(_dialect),
		m_nameDispenser(_dialect, _ast),
		m_typeInfo(_typeInfo)
	{}

	std::unique_ptr<wasm::Expression> visit(yul::Expression const& _expression);
	wasm::Expression visitReturnByValue(yul::Expression const& _expression);
	std::vector<wasm::Expression> visit(std::vector<yul::Expression> const& _expressions);
	wasm::Expression visit(yul::Statement const& _statement);
	std::vector<wasm::Expression> visit(std::vector<yul::Statement> const& _statements);

	/// Returns an assignment or a block containing multiple assignments.
	/// @param _variableNames the names of the variables to assign to
	/// @param _firstValue the value to be assigned to the first variable. If there
	///        is more than one variable, the values are taken from m_globalVariables.
	wasm::Expression generateMultiAssignment(
		std::vector<std::string> _variableNames,
		std::unique_ptr<wasm::Expression> _firstValue
	);

	wasm::FunctionDefinition translateFunction(yul::FunctionDefinition const& _funDef);

	/// Imports an external function into the current module.
	/// @param _builtin _builtin the builtin that will be imported into the current module.
	/// @param _module _module the module name under which the external function can be found.
	/// @param _externalName the name of the external function within the module _module.
	/// @param _internalName the name of the internal function under that the external function is accessible.
	void importBuiltinFunction(BuiltinFunction const* _builtin, std::string const& _module, std::string const& _externalName, std::string const& _internalName);

	std::string newLabel();
	/// Selects a subset of global variables matching specified sequence of variable types.
	/// Defines more global variables of a given type if there's not enough.
	std::vector<size_t> allocateGlobals(std::vector<wasm::Type> const& _typesForGlobals);

	static wasm::Type translatedType(yul::Type _yulType);
	static wasm::Literal makeLiteral(wasm::Type _type, u256 _value);

	Dialect const& m_dialect;
	NameDispenser m_nameDispenser;

	std::vector<wasm::VariableDeclaration> m_localVariables;
	std::vector<wasm::GlobalVariableDeclaration> m_globalVariables;
	std::map<YulString, wasm::FunctionImport> m_functionsToImport;
	std::string m_functionBodyLabel;
	std::stack<std::pair<std::string, std::string>> m_breakContinueLabelNames;
	TypeInfo& m_typeInfo;
};

}
