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
 * EWasm to binary encoder.
 */

#pragma once

#include <libyul/backends/wasm/EWasmAST.h>

#include <libdevcore/Common.h>

#include <vector>
#include <stack>

namespace yul
{
namespace wasm
{

/**
 * Web assembly to binary transform.
 */
class BinaryTransform
{
public:
	static dev::bytes run(Module const& _module);

	dev::bytes operator()(wasm::Literal const& _literal);
	dev::bytes operator()(wasm::StringLiteral const& _literal);
	dev::bytes operator()(wasm::LocalVariable const& _identifier);
	dev::bytes operator()(wasm::GlobalVariable const& _identifier);
	dev::bytes operator()(wasm::BuiltinCall const& _builinCall);
	dev::bytes operator()(wasm::FunctionCall const& _functionCall);
	dev::bytes operator()(wasm::LocalAssignment const& _assignment);
	dev::bytes operator()(wasm::GlobalAssignment const& _assignment);
	dev::bytes operator()(wasm::If const& _if);
	dev::bytes operator()(wasm::Loop const& _loop);
	dev::bytes operator()(wasm::Break const& _break);
	dev::bytes operator()(wasm::BreakIf const& _break);
	dev::bytes operator()(wasm::Block const& _block);
	dev::bytes operator()(wasm::FunctionDefinition const& _function);

private:
	using Type = std::pair<std::vector<std::uint8_t>, std::vector<std::uint8_t>>;
	static Type typeOf(wasm::FunctionImport const& _import);
	static Type typeOf(wasm::FunctionDefinition const& _funDef);

	static uint8_t encodeType(std::string const& _typeName);
	static std::vector<uint8_t> encodeTypes(std::vector<std::string> const& _typeNames);
	dev::bytes typeSection(
		std::vector<wasm::FunctionImport> const& _imports,
		std::vector<wasm::FunctionDefinition> const& _functions
	);

	dev::bytes importSection(std::vector<wasm::FunctionImport> const& _imports);
	dev::bytes functionSection(std::vector<wasm::FunctionDefinition> const& _functions);
	dev::bytes memorySection();
	dev::bytes globalSection();
	dev::bytes exportSection();
	dev::bytes customSection(std::string const& _name, dev::bytes _data);
	dev::bytes codeSection(std::vector<wasm::FunctionDefinition> const& _functions);

	dev::bytes visit(std::vector<wasm::Expression> const& _expressions);
	dev::bytes visitReversed(std::vector<wasm::Expression> const& _expressions);

	static dev::bytes encodeName(std::string const& _name);

	std::map<std::string, size_t> m_locals;
	std::map<std::string, size_t> m_globals;
	std::map<std::string, size_t> m_functions;
	std::map<std::string, size_t> m_functionTypes;
	std::stack<std::string> m_labels;
	std::map<std::string, std::pair<size_t, size_t>> m_subModulePosAndSize;
};


}
}

