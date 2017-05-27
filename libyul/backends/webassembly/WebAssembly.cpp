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
		m_assembly += "(module ";
		visitStatements(_block);
		m_assembly += ")";
	}

	string assembly() { return m_assembly; }

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
		if (_literal.kind == assembly::LiteralKind::Number)
			m_assembly += "(" + convertType(_literal.type) + ".const " + _literal.value + ")";
		else if (_literal.kind == assembly::LiteralKind::Boolean)
			m_assembly += "(" + convertType(_literal.type) + ".const " + string((_literal.value == "true") ? "1" : "0") + ")";
		else
			solUnimplementedAssert(false, "Non-number literals not supported.");
	}
	void operator()(assembly::Identifier const& _identifier)
	{
		m_assembly += "(get_local $" + _identifier.name + ")";
	}
	void operator()(assembly::VariableDeclaration const& _varDecl)
	{
		solUnimplementedAssert(_varDecl.variables.size() == 1, "Tuples not supported yet.");
		m_assembly += "(local $" + _varDecl.variables.front().name + " " + convertType(_varDecl.variables.front().type) + ")";
		m_assembly += "(set_local $" + _varDecl.variables.front().name + " ";
		boost::apply_visitor(*this, *_varDecl.value);
		m_assembly += ")";
	}
	void operator()(assembly::Assignment const& _assignment)
	{
		m_assembly += "(set_local $" + _assignment.variableName.name + " ";
		boost::apply_visitor(*this, *_assignment.value);
		m_assembly += ")";
	}
	void operator()(assembly::FunctionDefinition const& _funDef)
	{
		m_assembly += "(func $" + _funDef.name + " ";
		for (auto const& argument: _funDef.arguments)
			m_assembly += "(param $" + argument.name + " " + convertType(argument.type) + ")";
		solUnimplementedAssert(_funDef.returns.size() <= 1, "Multiple return values not supported yet.");
		string returnName;
		for (auto const& returnArgument: _funDef.returns)
		{
			returnName = returnArgument.name;
			m_assembly += "(result " + convertType(returnArgument.type) + ")";
			m_assembly += "(local $" + returnArgument.name + " " + convertType(returnArgument.type) + ")";
		}
		/// Scope rules: return parameters must be marked appropriately
		visitStatements(_funDef.body);
		if (!returnName.empty())
			m_assembly += "(return $" + returnName + ")";
		m_assembly += ")";
	}
	void operator()(assembly::FunctionCall const& _funCall)
	{
		m_assembly += "(call $" + _funCall.functionName.name;
		for (auto const& statement: _funCall.arguments)
		{
			m_assembly += " ";
			boost::apply_visitor(*this, statement);
		}
		m_assembly += ")";
	}
	void operator()(assembly::Switch const&)
	{
		solUnimplementedAssert(false, "Not implemented yet.");
	}
	void operator()(assembly::ForLoop const&)
	{
		solUnimplementedAssert(false, "Not implemented yet.");
	}
	void operator()(assembly::Block const& _block)
	{
		m_assembly += "(block ";
		visitStatements(_block);
		m_assembly += ")";
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

	string m_assembly;
};

string yul::WebAssembly::assemble(assembly::Block const& _block)
{
	return Generator(_block).assembly();
}
