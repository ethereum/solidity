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

#include <libyul/backends/wasm/BinaryTransform.h>
#include <libyul/backends/wasm/TextTransform.h>

#include <libyul/Exceptions.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Numeric.h>
#include <libsolutil/Keccak256.h>
#include <libsolutil/StringUtils.h>
#include <libsolutil/Visitor.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <range/v3/view/transform.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::yul::wasm;
using namespace solidity::util;

string TextTransform::run(wasm::Module const& _module)
{
	string ret = "(module\n";
	for (auto const& [name, module]: _module.subModules)
		ret +=
			"    ;; custom section for sub-module\n"
			"    ;; The Keccak-256 hash of the text representation of \"" +
			name +
			"\": " +
			toHex(keccak256(run(module))) +
			"\n"
			"    ;; (@custom \"" +
			name +
			"\" \"" +
			util::toHex(BinaryTransform::run(module)) +
			"\")\n";
	for (auto const& [name, data]: _module.customSections)
		ret +=
			"    ;; custom section for data\n"
			"    ;; (@custom \"" +
			name +
			"\" \"" +
			util::toHex(data) +
			"\")\n";
	for (wasm::FunctionImport const& imp: _module.imports)
	{
		ret += "    (import \"" + imp.module + "\" \"" + imp.externalName + "\" (func $" + imp.internalName;
		if (!imp.paramTypes.empty())
			ret += " (param" + joinHumanReadablePrefixed(imp.paramTypes | ranges::views::transform(encodeType), " ", " ") + ")";
		if (imp.returnType)
			ret += " (result " + encodeType(*imp.returnType) + ")";
		ret += "))\n";
	}

	// allocate one 64k page of memory and make it available to the Ethereum client
	ret += "    (memory $memory (export \"memory\") 1)\n";
	for (auto const& f: _module.functions)
		if (f.name == "main")
		{
			// export the main function
			ret += "    (export \"main\" (func $main))\n";
			break;
		}

	for (auto const& g: _module.globals)
		ret += "    (global $" + g.variableName + " (mut " + encodeType(g.type) + ") (" + encodeType(g.type) + ".const 0))\n";
	ret += "\n";
	for (auto const& f: _module.functions)
		ret += transform(f) + "\n";
	return move(ret) + ")\n";
}

string TextTransform::operator()(wasm::Literal const& _literal)
{
	return std::visit(GenericVisitor{
		[&](uint32_t _value) -> string { return "(i32.const " + to_string(_value) + ")"; },
		[&](uint64_t _value) -> string { return "(i64.const " + to_string(_value) + ")"; },
	}, _literal.value);
}

string TextTransform::operator()(wasm::StringLiteral const& _literal)
{
	// StringLiteral is a special AST element used for certain builtins.
	// The output of this will not be valid WebAssembly.
	string quoted = boost::replace_all_copy(_literal.value, "\\", "\\\\");
	boost::replace_all(quoted, "\"", "\\\"");
	return "\"" + quoted + "\"";
}

string TextTransform::operator()(wasm::LocalVariable const& _identifier)
{
	return "(local.get $" + _identifier.name + ")";
}

string TextTransform::operator()(wasm::GlobalVariable const& _identifier)
{
	return "(global.get $" + _identifier.name + ")";
}

string TextTransform::operator()(wasm::BuiltinCall const& _builtinCall)
{
	string args = joinTransformed(_builtinCall.arguments);
	string funcName = _builtinCall.functionName;
	// These are prefixed in the dialect, but are actually overloaded instructions in WebAssembly.
	if (funcName == "i32.drop" || funcName == "i64.drop")
		funcName = "drop";
	else if (funcName == "i32.select" || funcName == "i64.select")
		funcName = "select";
	return "(" + funcName + (args.empty() ? "" : " " + args) + ")";
}

string TextTransform::operator()(wasm::FunctionCall const& _functionCall)
{
	string args = joinTransformed(_functionCall.arguments);
	return "(call $" + _functionCall.functionName + (args.empty() ? "" : " " + args) + ")";
}

string TextTransform::operator()(wasm::LocalAssignment const& _assignment)
{
	return "(local.set $" + _assignment.variableName + " " + visit(*_assignment.value) + ")\n";
}

string TextTransform::operator()(wasm::GlobalAssignment const& _assignment)
{
	return "(global.set $" + _assignment.variableName + " " + visit(*_assignment.value) + ")\n";
}

string TextTransform::operator()(wasm::If const& _if)
{
	string text = "(if " + visit(*_if.condition) + " (then\n" + indented(joinTransformed(_if.statements, '\n')) + ")";
	if (_if.elseStatements)
		text += "(else\n" + indented(joinTransformed(*_if.elseStatements, '\n')) + ")";
	return std::move(text) + ")\n";
}

string TextTransform::operator()(wasm::Loop const& _loop)
{
	string label = _loop.labelName.empty() ? "" : " $" + _loop.labelName;
	return "(loop" + move(label) + "\n" + indented(joinTransformed(_loop.statements, '\n')) + ")\n";
}

string TextTransform::operator()(wasm::Branch const& _branch)
{
	return "(br $" + _branch.label.name + ")\n";
}

string TextTransform::operator()(wasm::BranchIf const& _branchIf)
{
	return "(br_if $" + _branchIf.label.name + " " + visit(*_branchIf.condition) + ")\n";
}

string TextTransform::operator()(wasm::Return const&)
{
	return "(return)\n";
}

string TextTransform::operator()(wasm::Block const& _block)
{
	string label = _block.labelName.empty() ? "" : " $" + _block.labelName;
	return "(block" + move(label) + "\n" + indented(joinTransformed(_block.statements, '\n')) + "\n)\n";
}

string TextTransform::indented(string const& _in)
{
	string replacement;

	if (!_in.empty())
	{
		replacement.reserve(_in.size() + 4);
		replacement += "    ";
		for (auto it = _in.begin(); it != _in.end(); ++it)
			if (*it == '\n' && it + 1 != _in.end() && *(it + 1) != '\n')
				replacement += "\n    ";
			else
				replacement += *it;
	}
	return replacement;
}

string TextTransform::transform(wasm::FunctionDefinition const& _function)
{
	string ret = "(func $" + _function.name + "\n";
	for (auto const& param: _function.parameters)
		ret += "    (param $" + param.name + " " + encodeType(param.type) + ")\n";
	if (_function.returnType.has_value())
		ret += "    (result " + encodeType(_function.returnType.value()) + ")\n";
	for (auto const& local: _function.locals)
		ret += "    (local $" + local.variableName + " " + encodeType(local.type) + ")\n";
	ret += indented(joinTransformed(_function.body, '\n'));
	if (ret.back() != '\n')
		ret += '\n';
	ret += ")\n";
	return ret;
}


string TextTransform::visit(wasm::Expression const& _expression)
{
	return std::visit(*this, _expression);
}

string TextTransform::joinTransformed(vector<wasm::Expression> const& _expressions, char _separator)
{
	string ret;
	for (auto const& e: _expressions)
	{
		string t = visit(e);
		if (!t.empty() && !ret.empty() && ret.back() != '\n')
			ret += _separator;
		ret += move(t);
	}
	return ret;
}

string TextTransform::encodeType(wasm::Type _type)
{
	if (_type == wasm::Type::i32)
		return "i32";
	else if (_type == wasm::Type::i64)
		return "i64";
	else
		yulAssert(false, "Invalid wasm type");
}
