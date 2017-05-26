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
 * @author Christian <c@ethdev.com>
 * @date 2017
 * Converts a parsed assembly into its textual form.
 */

#include <libsolidity/inlineasm/AsmPrinter.h>
#include <libsolidity/inlineasm/AsmData.h>
#include <libsolidity/interface/Utils.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <memory>
#include <functional>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::assembly;

//@TODO source locations

string AsmPrinter::operator()(assembly::Instruction const& _instruction)
{
	solAssert(!m_julia, "");
	return boost::to_lower_copy(instructionInfo(_instruction.instruction).name);
}

string AsmPrinter::operator()(assembly::Literal const& _literal)
{
	switch (_literal.kind)
	{
	case LiteralKind::Number:
		return _literal.value + appendTypeName(_literal.type);
	case LiteralKind::Boolean:
		return ((_literal.value == "true") ? "true" : "false") + appendTypeName(_literal.type);
	case LiteralKind::String:
		break;
	}

	string out;
	for (char c: _literal.value)
		if (c == '\\')
			out += "\\\\";
		else if (c == '"')
			out += "\\\"";
		else if (c == '\b')
			out += "\\b";
		else if (c == '\f')
			out += "\\f";
		else if (c == '\n')
			out += "\\n";
		else if (c == '\r')
			out += "\\r";
		else if (c == '\t')
			out += "\\t";
		else if (c == '\v')
			out += "\\v";
		else if (!isprint(c, locale::classic()))
		{
			ostringstream o;
			o << std::hex << setfill('0') << setw(2) << (unsigned)(unsigned char)(c);
			out += "\\x" + o.str();
		}
		else
			out += c;
	return "\"" + out + "\"" + appendTypeName(_literal.type);
}

string AsmPrinter::operator()(assembly::Identifier const& _identifier)
{
	return _identifier.name;
}

string AsmPrinter::operator()(assembly::FunctionalInstruction const& _functionalInstruction)
{
	solAssert(!m_julia, "");
	return
		(*this)(_functionalInstruction.instruction) +
		"(" +
		boost::algorithm::join(
			_functionalInstruction.arguments | boost::adaptors::transformed(boost::apply_visitor(*this)),
			", " ) +
		")";
}

string AsmPrinter::operator()(assembly::Label const& _label)
{
	solAssert(!m_julia, "");
	return _label.name + ":";
}

string AsmPrinter::operator()(assembly::StackAssignment const& _assignment)
{
	solAssert(!m_julia, "");
	return "=: " + (*this)(_assignment.variableName);
}

string AsmPrinter::operator()(assembly::Assignment const& _assignment)
{
	return (*this)(_assignment.variableName) + " := " + boost::apply_visitor(*this, *_assignment.value);
}

string AsmPrinter::operator()(assembly::VariableDeclaration const& _variableDeclaration)
{
	string out = "let ";
	out += boost::algorithm::join(
		_variableDeclaration.variables | boost::adaptors::transformed(
			[this](TypedName variable) { return variable.name + appendTypeName(variable.type); }
		),
		", "
	);
	out += " := ";
	out += boost::apply_visitor(*this, *_variableDeclaration.value);
	return out;
}

string AsmPrinter::operator()(assembly::FunctionDefinition const& _functionDefinition)
{
	string out = "function " + _functionDefinition.name + "(";
	out += boost::algorithm::join(
		_functionDefinition.arguments | boost::adaptors::transformed(
			[this](TypedName argument) { return argument.name + appendTypeName(argument.type); }
		),
		", "
	);
	out += ")";
	if (!_functionDefinition.returns.empty())
	{
		out += " -> ";
		out += boost::algorithm::join(
			_functionDefinition.returns | boost::adaptors::transformed(
				[this](TypedName argument) { return argument.name + appendTypeName(argument.type); }
			),
			", "
		);
	}

	return out + "\n" + (*this)(_functionDefinition.body);
}

string AsmPrinter::operator()(assembly::FunctionCall const& _functionCall)
{
	return
		(*this)(_functionCall.functionName) + "(" +
		boost::algorithm::join(
			_functionCall.arguments | boost::adaptors::transformed(boost::apply_visitor(*this)),
			", " ) +
		")";
}

string AsmPrinter::operator()(Switch const& _switch)
{
	string out = "switch " + boost::apply_visitor(*this, *_switch.expression);
	for (auto const& _case: _switch.cases)
	{
		if (!_case.value)
			out += "\ndefault ";
		else
			out += "\ncase " + (*this)(*_case.value) + " ";
		out += (*this)(_case.body);
	}
	return out;
}

string AsmPrinter::operator()(Block const& _block)
{
	if (_block.statements.empty())
		return "{\n}";
	string body = boost::algorithm::join(
		_block.statements | boost::adaptors::transformed(boost::apply_visitor(*this)),
		"\n"
	);
	boost::replace_all(body, "\n", "\n    ");
	return "{\n    " + body + "\n}";
}

string AsmPrinter::appendTypeName(std::string const& _type)
{
	if (m_julia)
		return ":" + _type;
	return "";
}
