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
 * Component that transforms internal Wasm representation to text.
 */

#pragma once

#include <libyul/backends/wasm/WasmAST.h>

#include <vector>

namespace solidity::yul
{
struct AsmAnalysisInfo;
}

namespace solidity::yul::wasm
{

class TextTransform
{
public:
	std::string run(wasm::Module const& _module);

public:
	std::string operator()(wasm::Literal const& _literal);
	std::string operator()(wasm::StringLiteral const& _literal);
	std::string operator()(wasm::LocalVariable const& _identifier);
	std::string operator()(wasm::GlobalVariable const& _identifier);
	std::string operator()(wasm::BuiltinCall const& _builinCall);
	std::string operator()(wasm::FunctionCall const& _functionCall);
	std::string operator()(wasm::LocalAssignment const& _assignment);
	std::string operator()(wasm::GlobalAssignment const& _assignment);
	std::string operator()(wasm::If const& _if);
	std::string operator()(wasm::Loop const& _loop);
	std::string operator()(wasm::Branch const& _branch);
	std::string operator()(wasm::BranchIf const& _branchIf);
	std::string operator()(wasm::Return const& _return);
	std::string operator()(wasm::Block const& _block);

private:
	std::string indented(std::string const& _in);

	std::string transform(wasm::FunctionDefinition const& _function);

	std::string visit(wasm::Expression const& _expression);
	std::string joinTransformed(
		std::vector<wasm::Expression> const& _expressions,
		char _separator = ' '
	);

	static std::string encodeType(wasm::Type _type);
};

}
