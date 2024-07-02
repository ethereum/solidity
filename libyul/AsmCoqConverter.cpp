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
 * @date 2019
 * Converts inline assembly AST to Coq format
 */

#include <libyul/AsmCoqConverter.h>
#include <libyul/AST.h>
#include <libyul/Exceptions.h>
#include <libyul/Utilities.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/UTF8.h>

namespace solidity::yul
{

std::string AsmCoqConverter::functions(Block const& _node)
{
	std::string ret = "[\n";
	m_indent++;
	ret += indent();

	bool isFirst = true;
	for (auto const& statement: _node.statements)
	{
		if (std::holds_alternative<FunctionDefinition>(statement))
		{
			if (!isFirst)
				ret += ";\n" + indent();
			isFirst = false;
			ret += "Code.Function.make (\n";
			m_indent++;
			ret += indent() + "\"" + std::get<FunctionDefinition>(statement).name.str() + "\",\n";
			ret += indent() + "[";
			bool isFirstInner = true;
			for (auto const& var: std::get<FunctionDefinition>(statement).parameters)
			{
				if (!isFirstInner)
					ret += "; ";
				isFirstInner = false;
				ret += "\"" + var.name.str() + "\"";
			}
			ret += "],\n";
			ret += indent() + "[";
			isFirstInner = true;
			for (auto const& var: std::get<FunctionDefinition>(statement).returnVariables)
			{
				if (!isFirstInner)
					ret += "; ";
				isFirstInner = false;
				ret += "\"" + var.name.str() + "\"";
			}
			ret += "],\n";
			ret += indent() + (*this)(std::get<FunctionDefinition>(statement).body) + "\n";
			m_indent--;
			ret += indent() + ")";
		}
	}

	ret += "\n";
	m_indent--;
	ret += indent() + "]";

	return ret;
}

std::string AsmCoqConverter::body(Block const& _node)
{
	std::string ret = "M.scope (\n";
	m_indent++;
	ret += indent();

	for (auto const& statement: _node.statements)
	{
		if (!std::holds_alternative<FunctionDefinition>(statement))
		{
			ret += "do! ltac:(M.monadic (\n";
			m_indent++;
			ret += indent() + std::visit(*this, statement) + "\n";
			m_indent--;
			ret += indent() + ")) in\n";
			ret += indent();
		}
	}

	ret += "M.pure BlockUnit.Tt\n";
	m_indent--;
	ret += indent() + ")";

	return ret;
}

std::string AsmCoqConverter::operator()(Block const& _node)
{
	std::string ret = "M.scope (\n";
	m_indent++;
	ret += indent();

	for (auto const& statement: _node.statements)
	{
		yulAssert(
			!std::holds_alternative<FunctionDefinition>(statement),
			"Function definitions are unhandled in blocks except at top-level"
		);
		ret += "do! ltac:(M.monadic (\n";
		m_indent++;
		ret += indent() + std::visit(*this, statement) + "\n";
		m_indent--;
		ret += indent() + ")) in\n";
		ret += indent();
	}

	ret += "M.pure BlockUnit.Tt\n";
	m_indent--;
	ret += indent() + ")";

	return ret;
}

std::string AsmCoqConverter::operator()(NameWithDebugData const& _node)
{
	yulAssert(!_node.name.empty(), "Invalid variable name.");
	std::string ret = "M.get_var (| \"" + _node.name.str() + "\" |)";

	return ret;
}

std::string AsmCoqConverter::operator()(Literal const& _node)
{
	return "[" + rawLiteral(_node) + "]";
}

std::string AsmCoqConverter::operator()(Identifier const& _node)
{
	yulAssert(!_node.name.empty(), "Invalid identifier");
	std::string ret = "M.get_var (| \"" + _node.name.str() + "\" |)";

	return ret;
}

std::string AsmCoqConverter::operator()(Assignment const& _node)
{
	yulAssert(_node.variableNames.size() >= 1, "Invalid assignment syntax");

	return rawAssign(false, _node.variableNames, _node.value);
}

std::string AsmCoqConverter::operator()(VariableDeclaration const& _node)
{
	return rawAssign(true, _node.variables, _node.value);
}

std::string AsmCoqConverter::operator()(FunctionCall const& _node)
{
	std::string ret = "M.call_function (|\n";
	m_indent++;
	ret += indent() + "\"" + _node.functionName.name.str() + "\",\n";
	if (_node.arguments.empty())
		ret += indent() + "[]\n";
	else
	{
		ret += indent() + "[\n";
		m_indent++;
		bool isFirst = true;
		for (auto const& argument: _node.arguments)
		{
			if (!isFirst)
				ret += ";\n";
			isFirst = false;
			ret += indent() + std::visit(*this, argument);
		}
		ret += "\n";
		m_indent--;
		ret += indent() + "]\n";
	}
	m_indent--;
	ret += indent() + "|)";

	return ret;
}

std::string AsmCoqConverter::operator()(ExpressionStatement const& _node)
{
	std::string ret = "M.expr_stmt (|\n";
	m_indent++;
	ret += indent() + std::visit(*this, _node.expression) + "\n";
	m_indent--;
	ret += indent() + "|)";

	return ret;
}

std::string AsmCoqConverter::operator()(FunctionDefinition const& _node)
{
	yulAssert(!_node.name.empty(), "Invalid function name.");
	std::string ret = "M.function (|\n";
	m_indent++;
	ret += indent() + "\"" + _node.name.str() + "\",\n";
	ret += indent() + "[";
	bool isFirst = true;
	for (auto const& var: _node.parameters)
	{
		if (!isFirst)
			ret += "; ";
		isFirst = false;
		ret += "\"" + var.name.str() + "\"";
	}
	ret += "],\n";
	ret += indent() + "[";
	isFirst = true;
	for (auto const& var: _node.returnVariables)
	{
		if (!isFirst)
			ret += "; ";
		isFirst = false;
		ret += "\"" + var.name.str() + "\"";
	}
	ret += "],\n";
	ret += indent() + (*this)(_node.body) + "\n";
	m_indent--;
	ret += indent() + "|)";

	return ret;
}

std::string AsmCoqConverter::operator()(If const& _node)
{
	yulAssert(_node.condition, "Invalid if condition.");
	std::string ret = "M.if_ (|\n";
	m_indent++;
	ret += indent() + std::visit(*this, *_node.condition) + ",\n";
	ret += indent() + (*this)(_node.body) + "\n";
	m_indent--;
	ret += indent() + "|)";

	return ret;
}

std::string AsmCoqConverter::operator()(Switch const& _node)
{
	yulAssert(_node.expression, "Invalid expression pointer.");
	std::string ret = "M.switch (|\n";
	m_indent++;
	ret += indent() + std::visit(*this, *_node.expression) + ",\n";
	ret += indent() + "[\n";
	m_indent++;
	bool isFirst = true;
	for (auto const& var: _node.cases)
	{
		if (!isFirst)
			ret += ";\n";
		isFirst = false;
		ret += indent() + (*this)(var);
	}
	ret += "\n";
	m_indent--;
	ret += indent() + "]\n";
	m_indent--;
	ret += indent() + "|)";

	return ret;
}

std::string AsmCoqConverter::operator()(Case const& _node)
{
	std::string ret = "(\n";
	m_indent++;
	ret += indent() + (_node.value ? "Some (" + rawLiteral(*_node.value) + ")" : "None") + ",\n";
	ret += indent() + (*this)(_node.body) + "\n";
	m_indent--;
	ret += indent() + ")";

	return ret;
}

std::string AsmCoqConverter::operator()(ForLoop const& _node)
{
	yulAssert(_node.condition, "Invalid for loop condition.");
	std::string ret = "do!\n";
	m_indent++;
	ret += indent() + (*this)(_node.pre) + " in\n";
	m_indent--;
	ret += indent() + "ltac:(M.monadic (\n";
	m_indent++;
	ret += indent() + "M.for_ (|\n";
	m_indent++;
	ret += indent() + "ltac:(M.monadic (\n";
	m_indent++;
	ret += indent() + std::visit(*this, *_node.condition) + "\n";
	m_indent--;
	ret += indent() + ")),\n";
	ret += indent() + (*this)(_node.post) + ",\n";
	ret += indent() + (*this)(_node.body) + "\n";
	m_indent--;
	ret += indent() + "|)\n";
	m_indent--;
	ret += indent() + "))";

	return ret;
}

std::string AsmCoqConverter::operator()(Break const& _node __attribute__((unused)))
{
	return "M.break (||)";
}

std::string AsmCoqConverter::operator()(Continue const& _node __attribute__((unused)))
{
	return "M.continue (||)";
}

std::string AsmCoqConverter::operator()(Leave const& _node __attribute__((unused)))
{
	return "M.leave (||)";
}

std::string AsmCoqConverter::indent() const
{
	return std::string(2 * m_indent, ' ');
}

std::string AsmCoqConverter::rawLiteral(Literal const& _node) const
{
	yulAssert(validLiteral(_node));
	std::string value = formatLiteral(_node);

	switch (_node.kind)
	{
	case LiteralKind::Number:
		yulAssert(
			util::isValidDecimal(value) || util::isValidHex(value),
			"Invalid number literal"
		);
		return "Literal.number " + value + "";
	case LiteralKind::Boolean:
		return "Literal.bool " + value + "";
	case LiteralKind::String:
	{
		std::string hex = util::toHex(util::asBytes(value));

		// Add a padding of zeros so that we always fit 32 bytes. The strings are in big-endian format.
		return "Literal.string 0x" + hex + std::string(64 - hex.size(), '0') + "";
	}
	default:
		yulAssert(false, "Invalid literal kind");
		return "invalid literal";
	}
}

template <class T>
std::string AsmCoqConverter::rawAssign(
	bool isDeclaration,
	std::vector<T> const& variables,
	std::unique_ptr<Expression> const& value
)
{
	std::string ret = isDeclaration ? "M.declare (|\n" : "M.assign (|\n";
	m_indent++;
	ret += indent() + "[";
	bool isFirst = true;
	for (auto const& var: variables)
	{
		if (!isFirst)
			ret += "; ";
		isFirst = false;
		ret += "\"" + var.name.str() + "\"";
	}
	ret += "],\n";
	ret += indent() + (value ? "Some (" + std::visit(*this, *value) + ")" : "None");
	m_indent--;
	ret += "\n" + indent() + "|)";

	return ret;
}

}
