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
 * Component that transforms internal Wasm representation to binary.
 */

#pragma once

#include <libyul/backends/wasm/WasmAST.h>

#include <libsolutil/Common.h>

#include <vector>
#include <stack>

namespace solidity::yul::wasm
{

/**
 * Web assembly to binary transform.
 */
class BinaryTransform
{
public:
	static bytes run(Module const& _module);

	bytes operator()(wasm::Literal const& _literal);
	bytes operator()(wasm::StringLiteral const& _literal);
	bytes operator()(wasm::LocalVariable const& _identifier);
	bytes operator()(wasm::GlobalVariable const& _identifier);
	bytes operator()(wasm::BuiltinCall const& _builinCall);
	bytes operator()(wasm::FunctionCall const& _functionCall);
	bytes operator()(wasm::LocalAssignment const& _assignment);
	bytes operator()(wasm::GlobalAssignment const& _assignment);
	bytes operator()(wasm::If const& _if);
	bytes operator()(wasm::Loop const& _loop);
	bytes operator()(wasm::Branch const& _branch);
	bytes operator()(wasm::BranchIf const& _branchIf);
	bytes operator()(wasm::Return const& _return);
	bytes operator()(wasm::Block const& _block);
	bytes operator()(wasm::FunctionDefinition const& _function);

private:
	BinaryTransform(
		std::map<std::string, size_t> _globalIDs,
		std::map<std::string, size_t> _functionIDs,
		std::map<std::string, size_t> _functionTypes,
		std::map<std::string, std::pair<size_t, size_t>> _subModulePosAndSize
	):
		m_globalIDs(std::move(_globalIDs)),
		m_functionIDs(std::move(_functionIDs)),
		m_functionTypes(std::move(_functionTypes)),
		m_subModulePosAndSize(std::move(_subModulePosAndSize))
	{}

	using Type = std::pair<std::vector<std::uint8_t>, std::vector<std::uint8_t>>;
	static Type typeOf(wasm::FunctionImport const& _import);
	static Type typeOf(wasm::FunctionDefinition const& _funDef);

	static uint8_t encodeType(wasm::Type _type);
	static std::vector<uint8_t> encodeTypes(std::vector<wasm::Type> const& _types);
	static std::vector<uint8_t> encodeTypes(wasm::TypedNameList const& _typedNameList);

	static std::map<Type, std::vector<std::string>> typeToFunctionMap(
		std::vector<wasm::FunctionImport> const& _imports,
		std::vector<wasm::FunctionDefinition> const& _functions
	);

	static std::map<std::string, size_t> enumerateGlobals(Module const& _module);
	static std::map<std::string, size_t> enumerateFunctions(Module const& _module);
	static std::map<std::string, size_t> enumerateFunctionTypes(
		std::map<Type, std::vector<std::string>> const& _typeToFunctionMap
	);

	static bytes typeSection(std::map<Type, std::vector<std::string>> const& _typeToFunctionMap);
	static bytes importSection(
		std::vector<wasm::FunctionImport> const& _imports,
		std::map<std::string, size_t> const& _functionTypes
	);
	static bytes functionSection(
		std::vector<wasm::FunctionDefinition> const& _functions,
		std::map<std::string, size_t> const& _functionTypes
	);
	static bytes memorySection();
	static bytes globalSection(std::vector<wasm::GlobalVariableDeclaration> const& _globals);
	static bytes exportSection(std::map<std::string, size_t> const& _functionIDs);
	static bytes customSection(std::string const& _name, bytes _data);
	bytes codeSection(std::vector<wasm::FunctionDefinition> const& _functions);

	bytes visit(std::vector<wasm::Expression> const& _expressions);
	bytes visitReversed(std::vector<wasm::Expression> const& _expressions);

	bytes encodeLabelIdx(std::string const& _label) const;

	static bytes encodeName(std::string const& _name);

	std::map<std::string, size_t> const m_globalIDs;
	std::map<std::string, size_t> const m_functionIDs;
	std::map<std::string, size_t> const m_functionTypes;
	/// The map of submodules, where the pair refers to the [offset, length]. The offset is
	/// an absolute offset within the resulting assembled bytecode.
	std::map<std::string, std::pair<size_t, size_t>> const m_subModulePosAndSize;

	std::map<std::string, size_t> m_locals;
	std::vector<std::string> m_labels;
};

}

