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
	bytes operator()(wasm::Break const& _break);
	bytes operator()(wasm::BreakIf const& _break);
	bytes operator()(wasm::Return const& _return);
	bytes operator()(wasm::Block const& _block);
	bytes operator()(wasm::FunctionDefinition const& _function);

private:
	using Type = std::pair<std::vector<std::uint8_t>, std::vector<std::uint8_t>>;
	static Type typeOf(wasm::FunctionImport const& _import);
	static Type typeOf(wasm::FunctionDefinition const& _funDef);

	static uint8_t encodeType(std::string const& _typeName);
	static std::vector<uint8_t> encodeTypes(std::vector<std::string> const& _typeNames);
	bytes typeSection(
		std::vector<wasm::FunctionImport> const& _imports,
		std::vector<wasm::FunctionDefinition> const& _functions
	);

	bytes importSection(std::vector<wasm::FunctionImport> const& _imports);
	bytes functionSection(std::vector<wasm::FunctionDefinition> const& _functions);
	bytes memorySection();
	bytes globalSection();
	bytes exportSection();
	bytes customSection(std::string const& _name, bytes _data);
	bytes codeSection(std::vector<wasm::FunctionDefinition> const& _functions);

	bytes visit(std::vector<wasm::Expression> const& _expressions);
	bytes visitReversed(std::vector<wasm::Expression> const& _expressions);

	bytes encodeLabelIdx(std::string const& _label) const;

	static bytes encodeName(std::string const& _name);

	std::map<std::string, size_t> m_locals;
	std::map<std::string, size_t> m_globals;
	std::map<std::string, size_t> m_functions;
	std::map<std::string, size_t> m_functionTypes;
	std::vector<std::string> m_labels;
	std::map<std::string, std::pair<size_t, size_t>> m_subModulePosAndSize;
};

}

