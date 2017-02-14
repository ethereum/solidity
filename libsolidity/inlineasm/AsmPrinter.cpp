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
	return boost::to_lower_copy(instructionInfo(_instruction.instruction).name);
}

string AsmPrinter::operator()(assembly::Literal const& _literal)
{
	if (_literal.isNumber)
		return _literal.value;
	string out;
	for (char c: _literal.value)
		if (c == '\\')
			out += "\\\\";
		else if (c == '"')
			out += "\\";
		else if (c == '\'')
			out += "\\'";
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
			o << std::hex << setfill('0') << setw(2) << unsigned(c);
			out += "\\x" + o.str();
		}
		else
			out += c;
	return "\"" + out + "\"";
}

string AsmPrinter::operator()(assembly::Identifier const& _identifier)
{
	return _identifier.name;
}

string AsmPrinter::operator()(assembly::FunctionalInstruction const& _functionalInstruction)
{
	return
		(*this)(_functionalInstruction.instruction); +
		"(" +
		boost::algorithm::join(
			_functionalInstruction.arguments | boost::adaptors::transformed(boost::apply_visitor(*this)),
			", " ) +
		")";
}

string AsmPrinter::operator()(assembly::Label const& _label)
{
	return _label.name + ":";
}

string AsmPrinter::operator()(assembly::Assignment const& _assignment)
{
	return "=: " + (*this)(_assignment.variableName);
}

string AsmPrinter::operator()(assembly::FunctionalAssignment const& _functionalAssignment)
{
	return (*this)(_functionalAssignment.variableName) + " := " + boost::apply_visitor(*this, *_functionalAssignment.value);
}

string AsmPrinter::operator()(assembly::VariableDeclaration const& _variableDeclaration)
{
	return "let " + _variableDeclaration.name + " := " + boost::apply_visitor(*this, *_variableDeclaration.value);
}

string AsmPrinter::operator()(Block const& _block)
{
	string body = boost::algorithm::join(
		_block.statements | boost::adaptors::transformed(boost::apply_visitor(*this)),
		"\n"
	);
	boost::replace_all(body, "\n", "\n    ");
	return "{\n    " + body + "\n}";
}
