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
 * @author Alex Beregszaszi
 * @date 2017
 * Yul to WebAssembly code generator.
 */

#include <libyul/backends/webassembly/WebAssembly.h>
#include <libdevcore/IndentedWriter.h>
#include <libsolidity/inlineasm/AsmData.h>
#include <libsolidity/interface/Exceptions.h>

#include <libdevcore/CommonIO.h>

#include <binaryen-c.h>
#include <wasm-builder.h>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/count_if.hpp>

#include <memory>
#include <functional>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::assembly;

class Generator: public boost::static_visitor<>
{
public:
	/// Create the code transformer which appends assembly to _state.assembly when called
	/// with parsed assembly data.
	/// @param _identifierAccess used to resolve identifiers external to the inline assembly
	explicit Generator(assembly::Block const& _block)
	{
		m_output.addLine("(module ");
		m_output.indent();
		visitStatements(_block);
		m_output.unindent();
		m_output.addLine(")");
	}

	string assembly() { return m_output.format(); }

public:
	void operator()(assembly::Instruction const&)
	{
		solAssert(false, "Instructions are not supported in Yul.");
	}
	void operator()(assembly::FunctionalInstruction const&)
	{
		solAssert(false, "Instructions are not supported in Yul.");
	}
	void operator()(assembly::StackAssignment const&)
	{
		solAssert(false, "Assignment from stack is not supported in Yul.");
	}
	void operator()(assembly::Label const&)
	{
		solAssert(false, "Labels are not supported in Yul.");
	}
	void operator()(assembly::Literal const& _literal)
	{
		solAssert(!m_onRoot, "Statements are only allowed within functions.");

		if (_literal.kind == assembly::LiteralKind::Number)
			m_output.add("(" + convertType(_literal.type) + ".const " + _literal.value + ")");
		else if (_literal.kind == assembly::LiteralKind::Boolean)
			m_output.add("(" + convertType(_literal.type) + ".const " + string((_literal.value == "true") ? "1" : "0") + ")");
		else
			solUnimplementedAssert(false, "Non-number literals not supported.");
	}
	void operator()(assembly::Identifier const& _identifier)
	{
		solAssert(!m_onRoot, "Statements are only allowed within functions.");

		m_output.add("(get_local $" + _identifier.name + ")");
	}
	void operator()(assembly::VariableDeclaration const& _varDecl)
	{
		solAssert(!m_onRoot, "Statements are only allowed within functions.");

		solUnimplementedAssert(_varDecl.variables.size() == 1, "Tuples not supported yet.");
		m_output.addLine("(local $" + _varDecl.variables.front().name + " " + convertType(_varDecl.variables.front().type) + ")");
		m_output.addLine("(set_local $" + _varDecl.variables.front().name + " ");
		m_output.indent();
		boost::apply_visitor(*this, *_varDecl.value);
		m_output.unindent();
		m_output.add(")");
		m_output.newLine();
	}
	void operator()(assembly::Assignment const& _assignment)
	{
		solAssert(!m_onRoot, "Statements are only allowed within functions.");

		m_output.addLine("(set_local $" + _assignment.variableName.name + " ");
		m_output.indent();
		boost::apply_visitor(*this, *_assignment.value);
		m_output.unindent();
		m_output.add(")");
		m_output.newLine();
	}
	void operator()(assembly::FunctionDefinition const& _funDef)
	{
		solAssert(m_onRoot, "Embedded functions are not supported.");

		m_onRoot = false;
		m_output.newLine();
		m_output.addLine("(func $" + _funDef.name + " ");
		m_output.indent();
		for (auto const& argument: _funDef.arguments)
			m_output.addLine("(param $" + argument.name + " " + convertType(argument.type) + ")");
		solUnimplementedAssert(_funDef.returns.size() <= 1, "Multiple return values not supported yet.");
		string returnName;
		for (auto const& returnArgument: _funDef.returns)
		{
			returnName = returnArgument.name;
			m_output.addLine("(result " + convertType(returnArgument.type) + ")");
			m_output.addLine("(local $" + returnArgument.name + " " + convertType(returnArgument.type) + ")");
		}
		/// Scope rules: return parameters must be marked appropriately
		m_output.newLine();
		m_output.newLine();
		visitStatements(_funDef.body);
		m_output.newLine();
		m_output.newLine();
		if (!returnName.empty())
			m_output.addLine("(return $" + returnName + ")");
		m_output.unindent();
		m_output.addLine(")");
		m_output.newLine();
		m_onRoot = true;
	}
	void operator()(assembly::FunctionCall const& _funCall)
	{
		solAssert(!m_onRoot, "Statements are only allowed within functions.");

		if (resolveBuiltinFunction(_funCall))
			return;

		m_output.addLine("(call $" + _funCall.functionName.name);
		m_output.indent();
		for (auto const& statement: _funCall.arguments)
		{
			m_output.add(" ");
			boost::apply_visitor(*this, statement);
			m_output.newLine();
		}
		m_output.unindent();
		m_output.addLine(")");
	}
	void operator()(assembly::Switch const& _switch)
	{
		solAssert(!m_onRoot, "Statements are only allowed within functions.");

		solUnimplementedAssert(_switch.cases.size() <= 2, "");
		/// One of the cases must be the default case
		solUnimplementedAssert(
			_switch.cases[0].value ||
			_switch.cases[1].value,
			""
		);
		unsigned defaultcase = _switch.cases[0].value ? 0 : 1;
		solUnimplementedAssert(defaultcase <= (_switch.cases.size() - 1), "");

		m_output.addLine("(if (result i64) ");
		m_output.indent();
		m_output.add("(i64.eq ");
		boost::apply_visitor(*this, *_switch.expression);
		m_output.add(" ");
		(*this)(*(_switch.cases[!!defaultcase].value));
		m_output.add(")");
		m_output.newLine();
		m_output.add("(then ");
		m_output.indent();
		(*this)(_switch.cases[!!defaultcase].body);
		m_output.unindent();
		m_output.addLine(")");
		if (_switch.cases.size() == 2)
		{
			m_output.add("(else ");
			m_output.indent();
			(*this)(_switch.cases[defaultcase].body);
			m_output.unindent();
			m_output.addLine(")");
		}
		m_output.unindent();
		m_output.addLine(")");
	}
	void operator()(assembly::ForLoop const&)
	{
		solUnimplementedAssert(false, "Not implemented yet.");
	}
	void operator()(assembly::Block const& _block)
	{
		solAssert(!m_onRoot, "Statements are only allowed within functions.");

		m_output.add("(block ");
		m_output.indent();
		visitStatements(_block);
		m_output.unindent();
		m_output.add(")");
	}
private:
	void visitStatements(assembly::Block const& _block)
	{
		std::for_each(_block.statements.begin(), _block.statements.end(), boost::apply_visitor(*this));
	}

	string convertType(assembly::Type type)
	{
		solAssert(!type.empty(), "Only Julia input is supported.");
		set<string> const supportedTypes{"bool", "u8", "s8", "u32", "s32", "u64", "s64"};
		solAssert(supportedTypes.count(type), "Type (" + type + ") not supported yet.");
		return "i64";
	}

	/// TODO: replace with a proper structure (and not manual code)
	bool resolveBuiltinFunction(assembly::FunctionCall const& _funCall)
	{
		if (_funCall.functionName.name == "add64")
		{
			m_output.add("(i64.add ");
			m_output.indent();
			solAssert(_funCall.arguments.size() == 2, "");
			boost::apply_visitor(*this, _funCall.arguments[0]);
			m_output.newLine();
			boost::apply_visitor(*this, _funCall.arguments[1]);
			m_output.unindent();
			m_output.add(")");
			return true;
		}
		else if (_funCall.functionName.name == "sub64")
		{
			m_output.add("(i64.sub ");
			m_output.indent();
			solAssert(_funCall.arguments.size() == 2, "");
			boost::apply_visitor(*this, _funCall.arguments[0]);
			m_output.newLine();
			boost::apply_visitor(*this, _funCall.arguments[1]);
			m_output.unindent();
			m_output.add(")");
			return true;
		}
		else if (_funCall.functionName.name == "mul64")
		{
			m_output.add("(i64.mul ");
			m_output.indent();
			solAssert(_funCall.arguments.size() == 2, "");
			boost::apply_visitor(*this, _funCall.arguments[0]);
			m_output.newLine();
			boost::apply_visitor(*this, _funCall.arguments[1]);
			m_output.unindent();
			m_output.add(")");
			return true;
		}
		else if (_funCall.functionName.name == "gt64")
		{
			m_output.add("(i64.gt_u ");
			m_output.indent();
			solAssert(_funCall.arguments.size() == 2, "");
			boost::apply_visitor(*this, _funCall.arguments[0]);
			m_output.newLine();
			boost::apply_visitor(*this, _funCall.arguments[1]);
			m_output.unindent();
			m_output.add(")");
			return true;
		}

		return false;
	}

	IndentedWriter m_output;
	bool m_onRoot = true;
};

string yul::WebAssembly::assemble(assembly::Block const& _block)
{
#if 0
	BinaryenModuleRef module = BinaryenModuleCreate();

	// Create a function type for  i32 (i32, i32)
	BinaryenType params[2] = { BinaryenInt32(), BinaryenInt32() };
	BinaryenFunctionTypeRef iii = BinaryenAddFunctionType(module, "iii", BinaryenInt32(), params, 2);

	// Get the 0 and 1 arguments, and add them
	BinaryenExpressionRef x = BinaryenGetLocal(module, 0, BinaryenInt32()),
                        y = BinaryenGetLocal(module, 1, BinaryenInt32());
	BinaryenExpressionRef add = BinaryenBinary(module, BinaryenAddInt32(), x, y);

	// Create the add function
	// Note: no additional local variables
	// Note: no basic blocks here, we are an AST. The function body is just an expression node.
	BinaryenFunctionRef adder = BinaryenAddFunction(module, "adder", iii, NULL, 0, add);

	BinaryenSetFunctionTable(module, &adder, 1);

	// Print it out
	BinaryenModulePrint(module);

	// Clean up the module, which owns all the objects we created above
	BinaryenModuleDispose(module);
#endif

	return Generator(_block).assembly();
}
