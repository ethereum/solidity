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
 * Component that transforms interval EWasm representation to text.
 */

#include <libyul/backends/wasm/EWasmToText.h>

#include <libdevcore/StringUtils.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/range/adaptor/transformed.hpp>

using namespace std;
using namespace yul;
using namespace dev;

string EWasmToText::run(
	vector<wasm::GlobalVariableDeclaration> const& _globals,
	vector<wasm::FunctionImport> const& _imports,
	vector<wasm::FunctionDefinition> const& _functions
)
{
	string ret = "(module\n";
	for (wasm::FunctionImport const& imp: _imports)
	{
		ret += "    (import \"" + imp.module + "\" \"" + imp.externalName + "\" (func $" + imp.internalName;
		if (!imp.paramTypes.empty())
			ret += " (param" + joinHumanReadablePrefixed(imp.paramTypes, " ", " ") + ")";
		if (imp.returnType)
			ret += " (result " + *imp.returnType + ")";
		ret += "))\n";
	}

	// allocate one 64k page of memory and make it available to the Ethereum client
	ret += "    (memory $memory (export \"memory\") 1)\n";
	// export the main function
	ret += "    (export \"main\" (func $main))\n";

	for (auto const& g: _globals)
		ret += "    (global $" + g.variableName + " (mut i64) (i64.const 0))\n";
	ret += "\n";
	for (auto const& f: _functions)
		ret += transform(f) + "\n";
	return move(ret) + ")\n";
}

string EWasmToText::operator()(wasm::Literal const& _literal)
{
	return "(i64.const " + to_string(_literal.value) + ")";
}

string EWasmToText::operator()(wasm::StringLiteral const& _literal)
{
	string quoted = boost::replace_all_copy(_literal.value, "\\", "\\\\");
	boost::replace_all(quoted, "\"", "\\\"");
	return "\"" + quoted + "\"";
}

string EWasmToText::operator()(wasm::LocalVariable const& _identifier)
{
	return "(get_local $" + _identifier.name + ")";
}

string EWasmToText::operator()(wasm::GlobalVariable const& _identifier)
{
	return "(get_global $" + _identifier.name + ")";
}

string EWasmToText::operator()(wasm::BuiltinCall const& _builtinCall)
{
	string args = joinTransformed(_builtinCall.arguments);
	return "(" + _builtinCall.functionName + (args.empty() ? "" : " " + args) + ")";
}

string EWasmToText::operator()(wasm::FunctionCall const& _functionCall)
{
	string args = joinTransformed(_functionCall.arguments);
	return "(call $" + _functionCall.functionName + (args.empty() ? "" : " " + args) + ")";
}

string EWasmToText::operator()(wasm::LocalAssignment const& _assignment)
{
	return "(set_local $" + _assignment.variableName + " " + visit(*_assignment.value) + ")\n";
}

string EWasmToText::operator()(wasm::GlobalAssignment const& _assignment)
{
	return "(set_global $" + _assignment.variableName + " " + visit(*_assignment.value) + ")\n";
}

string EWasmToText::operator()(wasm::If const& _if)
{
	string text = "(if " + visit(*_if.condition) + " (then\n" + indented(joinTransformed(_if.statements, '\n')) + ")";
	if (_if.elseStatements)
		text += "(else\n" + indented(joinTransformed(*_if.elseStatements, '\n')) + ")";
	return std::move(text) + ")\n";
}

string EWasmToText::operator()(wasm::Loop const& _loop)
{
	string label = _loop.labelName.empty() ? "" : " $" + _loop.labelName;
	return "(loop" + move(label) + "\n" + indented(joinTransformed(_loop.statements, '\n')) + ")\n";
}

string EWasmToText::operator()(wasm::Break const& _break)
{
	return "(break $" + _break.label.name + ")\n";
}

string EWasmToText::operator()(wasm::BreakIf const& _break)
{
	return "(br_if $" + _break.label.name + " " + visit(*_break.condition) + ")\n";
}

string EWasmToText::operator()(wasm::Block const& _block)
{
	string label = _block.labelName.empty() ? "" : " $" + _block.labelName;
	return "(block" + move(label) + "\n" + indented(joinTransformed(_block.statements, '\n')) + "\n)\n";
}

string EWasmToText::indented(string const& _in)
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

string EWasmToText::transform(wasm::FunctionDefinition const& _function)
{
	string ret = "(func $" + _function.name + "\n";
	for (auto const& param: _function.parameterNames)
		ret += "    (param $" + param + " i64)\n";
	if (_function.returns)
		ret += "    (result i64)\n";
	for (auto const& local: _function.locals)
		ret += "    (local $" + local.variableName + " i64)\n";
	ret += indented(joinTransformed(_function.body, '\n'));
	if (ret.back() != '\n')
		ret += '\n';
	ret += ")\n";
	return ret;
}


string EWasmToText::visit(wasm::Expression const& _expression)
{
	return boost::apply_visitor(*this, _expression);
}

string EWasmToText::joinTransformed(vector<wasm::Expression> const& _expressions, char _separator)
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
