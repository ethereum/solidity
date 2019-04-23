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

string EWasmToText::run(
	vector<wasm::GlobalVariableDeclaration> const& _globals,
	vector<wasm::FunctionDefinition> const& _functions
)
{
	string ret = "(module\n\n";
	for (auto const& g: _globals)
		ret += "    (global $" + g.variableName + " (mut i64) (i64.const 0))\n";
	for (auto const& f: _functions)
		ret += transform(f) + "\n";
	return move(ret) + ")\n";
}

string EWasmToText::operator()(wasm::Literal const& _literal)
{
	return "(i64.const " + to_string(_literal.value) + ")";
}

string EWasmToText::operator()(wasm::LocalVariable const& _identifier)
{
	return "(get_local $" + _identifier.name + ")";
}

string EWasmToText::operator()(wasm::GlobalVariable const& _identifier)
{
	return "(get_global $" + _identifier.name + ")";
}

string EWasmToText::operator()(wasm::Label const& _label)
{
	return "$" + _label.name;
}

string EWasmToText::operator()(wasm::BuiltinCall const& _builtinCall)
{
	return "(" + _builtinCall.functionName + " " + joinTransformed(_builtinCall.arguments) + ")";
}

string EWasmToText::operator()(wasm::FunctionCall const& _functionCall)
{
	return "(call $" + _functionCall.functionName + " " + joinTransformed(_functionCall.arguments) + ")";
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
	return "(if " + visit(*_if.condition) + " (then\n" + indented(joinTransformed(_if.statements)) + "\n))\n";
}

string EWasmToText::operator()(wasm::Loop const& _loop)
{
	string label = _loop.labelName.empty() ? "" : " $" + _loop.labelName;
	return "(loop" + move(label) + "\n" + indented(joinTransformed(_loop.statements)) + ")\n";
}

string EWasmToText::operator()(wasm::Break const& _break)
{
	return "(break $" + _break.label.name + ")\n";
}

string EWasmToText::operator()(wasm::Continue const& _continue)
{
	return "(continue $" + _continue.label.name + ")\n";
}

string EWasmToText::operator()(wasm::Block const& _block)
{
	string label = _block.labelName.empty() ? "" : " $" + _block.labelName;
	return "(block" + move(label) + "\n" + indented(joinTransformed(_block.statements)) + "\n)\n";
}

string EWasmToText::indented(string const& _in)
{
	string replacement;
	if (!_in.empty())
	{
		replacement = "    " + boost::replace_all_copy(_in, "\n", "\n    ");
		if (_in.back() == '\n')
			replacement = replacement.substr(0, replacement.size() - 4);
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
	ret += indented(joinTransformed(_function.body));
	if (ret.back() != '\n')
		ret += '\n';
	ret += ")\n";
	return ret;
}


string EWasmToText::visit(wasm::Expression const& _expression)
{
	return boost::apply_visitor(*this, _expression);
}

string EWasmToText::joinTransformed(vector<wasm::Expression> const& _expressions)
{
	string ret;
	for (auto const& e: _expressions)
	{
		string t = visit(e);
		if (!t.empty() && !ret.empty() && ret.back() != '\n')
			ret += ' ';
		ret += move(t);
	}
	return ret;
}
