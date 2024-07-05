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

#include <libsolidity/formal/ExpressionFormatter.h>

#include <libsolidity/formal/SymbolicTypes.h>

#include <libsolutil/Algorithms.h>
#include <libsolutil/CommonData.h>

#include <boost/algorithm/string.hpp>

#include <map>
#include <vector>
#include <string>

using boost::algorithm::starts_with;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::smtutil;
using namespace solidity::frontend::smt;

namespace solidity::frontend::smt
{

namespace
{

std::string formatDatatypeAccessor(smtutil::Expression const& _expr, std::vector<std::string> const& _args)
{
	auto const& op = _expr.name;

	// This is the most complicated part of the translation.
	// Datatype accessor means access to a field of a datatype.
	// In our encoding, datatypes are used to encode:
	// - arrays/mappings as the tuple (array, length)
	// - structs as the tuple (<member1>, ..., <memberK>)
	// - hash and signature functions as the tuple (keccak256, sha256, ripemd160, ecrecover),
	//   where each element is an array emulating an UF
	// - abi.* functions as the tuple (<abiCall1>, ..., <abiCallK>).
	if (op == "dt_accessor_keccak256")
		return "keccak256";
	if (op == "dt_accessor_sha256")
		return "sha256";
	if (op == "dt_accessor_ripemd160")
		return "ripemd160";
	if (op == "dt_accessor_ecrecover")
		return "ecrecover";

	std::string accessorStr = "accessor_";
	// Struct members have suffix "accessor_<memberName>".
	std::string type = op.substr(op.rfind(accessorStr) + accessorStr.size());
	solAssert(_expr.arguments.size() == 1, "");

	if (type == "length")
		return _args.at(0) + ".length";
	if (type == "array")
		return _args.at(0);

	if (
		starts_with(type, "block") ||
		starts_with(type, "msg") ||
		starts_with(type, "tx") ||
		starts_with(type, "abi")
	)
		return type;

	if (starts_with(type, "t_function_abi"))
		return type;

	return _args.at(0) + "." + type;
}

std::string formatGenericOp(smtutil::Expression const& _expr, std::vector<std::string> const& _args)
{
	return _expr.name + "(" + boost::algorithm::join(_args, ", ") + ")";
}

std::string formatInfixOp(std::string const& _op, std::vector<std::string> const& _args)
{
	return "(" + boost::algorithm::join(_args, " " + _op + " ") + ")";
}

std::string formatArrayOp(smtutil::Expression const& _expr, std::vector<std::string> const& _args)
{
	if (_expr.name == "select")
	{
		auto const& a0 = _args.at(0);
		static std::set<std::string> const ufs{"keccak256", "sha256", "ripemd160", "ecrecover"};
		if (ufs.count(a0) || starts_with(a0, "t_function_abi"))
			return _args.at(0) + "(" + _args.at(1) + ")";
		return _args.at(0) + "[" + _args.at(1) + "]";
	}
	if (_expr.name == "store")
		return "(" + _args.at(0) + "[" + _args.at(1) + "] := " + _args.at(2) + ")";
	return formatGenericOp(_expr, _args);
}

std::string formatUnaryOp(smtutil::Expression const& _expr, std::vector<std::string> const& _args)
{
	if (_expr.name == "not")
		return "!" + _args.at(0);
	if (_expr.name == "-")
		return "-" + _args.at(0);
	// Other operators such as exists may end up here.
	return formatGenericOp(_expr, _args);
}

}

smtutil::Expression substitute(smtutil::Expression _from, std::map<std::string, std::string> const& _subst)
{
	// TODO For now we ignore nested quantifier expressions,
	// but we should support them in the future.
	if (_from.name == "forall" || _from.name == "exists")
		return smtutil::Expression(true);
	if (_subst.count(_from.name))
		_from.name = _subst.at(_from.name);
	for (auto& arg: _from.arguments)
		arg = substitute(arg, _subst);
	return _from;
}

std::string toSolidityStr(smtutil::Expression const& _expr)
{
	auto const& op = _expr.name;

	auto const& args = _expr.arguments;
	auto strArgs = util::applyMap(args, [](auto const& _arg) { return toSolidityStr(_arg); });

	// Constant or variable.
	if (args.empty())
		return op;

	if (starts_with(op, "dt_accessor"))
		return formatDatatypeAccessor(_expr, strArgs);

	// Infix operators with format replacements.
	static std::map<std::string, std::string> const infixOps{
		{"and", "&&"},
		{"or", "||"},
		{"implies", "=>"},
		{"=", "="},
		{">", ">"},
		{">=", ">="},
		{"<", "<"},
		{"<=", "<="},
		{"+", "+"},
		{"-", "-"},
		{"*", "*"},
		{"/", "/"},
		{"div", "/"},
		{"mod", "%"}
	};
	// Some of these (and, or, +, *) may have >= 2 arguments from z3.
	if (infixOps.count(op) && args.size() >= 2)
		return formatInfixOp(infixOps.at(op), strArgs);

	static std::set<std::string> const arrayOps{"select", "store", "const_array"};
	if (arrayOps.count(op))
		return formatArrayOp(_expr, strArgs);

	if (args.size() == 1)
		return formatUnaryOp(_expr, strArgs);

	// Other operators such as bv2int, int2bv may end up here.
	return op + "(" + boost::algorithm::join(strArgs, ", ") + ")";
}

namespace
{
bool fillArray(smtutil::Expression const& _expr, std::vector<std::string>& _array, ArrayType const& _type)
{
	// Base case
	if (_expr.name == "const_array")
	{
		auto length = _array.size();
		std::optional<std::string> elemStr = expressionToString(_expr.arguments.at(1), _type.baseType());
		if (!elemStr)
			return false;
		_array.clear();
		_array.resize(length, *elemStr);
		return true;
	}

	// Recursive case.
	if (_expr.name == "store")
	{
		if (!fillArray(_expr.arguments.at(0), _array, _type))
			return false;
		std::optional<std::string> indexStr = expressionToString(_expr.arguments.at(1), TypeProvider::uint256());
		if (!indexStr)
			return false;
		// Sometimes the solver assigns huge lengths that are not related,
		// we should catch and ignore those.
		unsigned long index;
		try
		{
			index = stoul(*indexStr);
		}
		catch (std::out_of_range const&)
		{
			return true;
		}
		catch (std::invalid_argument const&)
		{
			return true;
		}
		std::optional<std::string> elemStr = expressionToString(_expr.arguments.at(2), _type.baseType());
		if (!elemStr)
			return false;
		if (index < _array.size())
			_array.at(index) = *elemStr;
		return true;
	}

	// Special base case, not supported yet.
	if (_expr.name.rfind("(_ as-array") == 0)
	{
		// Z3 expression representing reinterpretation of a different term as an array
		return false;
	}

	solAssert(false);
}
}

std::optional<std::string> expressionToString(smtutil::Expression const& _expr, frontend::Type const* _type)
{
	if (smt::isNumber(*_type))
	{
		solAssert(_expr.sort->kind == Kind::Int);
		solAssert(_expr.arguments.empty() || _expr.name == "-");
		if (_expr.name == "-")
		{
			solAssert(_expr.arguments.size() == 1);
			smtutil::Expression const& val = _expr.arguments[0];
			solAssert(val.sort->kind == Kind::Int && val.arguments.empty());
			return "(- " + val.name + ")";
		}

		if (
			_type->category() == frontend::Type::Category::Address ||
			_type->category() == frontend::Type::Category::FixedBytes
		)
		{
			try
			{
				if (_expr.name == "0")
					return "0x0";
				// For some reason the code below returns "0x" for "0".
				return util::toHex(toCompactBigEndian(bigint(_expr.name)), util::HexPrefix::Add, util::HexCase::Lower);
			}
			catch (std::out_of_range const&)
			{
			}
			catch (std::invalid_argument const&)
			{
			}
		}

		return _expr.name;
	}
	if (smt::isBool(*_type))
	{
		solAssert(_expr.sort->kind == Kind::Bool);
		solAssert(_expr.arguments.empty());
		solAssert(_expr.name == "true" || _expr.name == "false");
		return _expr.name;
	}
	if (smt::isFunction(*_type))
	{
		solAssert(_expr.arguments.empty());
		return _expr.name;
	}
	if (smt::isArray(*_type))
	{
		auto const& arrayType = dynamic_cast<ArrayType const&>(*_type);
		if (_expr.name != "tuple_constructor")
			return {};

		auto const& tupleSort = dynamic_cast<TupleSort const&>(*_expr.sort);
		solAssert(tupleSort.components.size() == 2);

		unsigned long length;
		try
		{
			length = stoul(_expr.arguments.at(1).name);
		}
		catch(std::out_of_range const&)
		{
			return {};
		}
		catch(std::invalid_argument const&)
		{
			return {};
		}

		// Limit this counterexample size to 1k.
		// Some OSs give you "unlimited" memory through swap and other virtual memory,
		// so purely relying on bad_alloc being thrown is not a good idea.
		// In that case, the array allocation might cause OOM and the program is killed.
		if (length >= 1024)
			return {};
		try
		{
			std::vector<std::string> array(length);
			if (!fillArray(_expr.arguments.at(0), array, arrayType))
				return {};
			return "[" + boost::algorithm::join(array, ", ") + "]";
		}
		catch (std::bad_alloc const&)
		{
			// Solver gave a concrete array but length is too large.
		}
	}
	if (smt::isNonRecursiveStruct(*_type))
	{
		auto const& structType = dynamic_cast<StructType const&>(*_type);
		solAssert(_expr.name == "tuple_constructor");
		auto const& tupleSort = dynamic_cast<TupleSort const&>(*_expr.sort);
		auto members = structType.structDefinition().members();
		solAssert(tupleSort.components.size() == members.size());
		solAssert(_expr.arguments.size() == members.size());
		std::vector<std::string> elements;
		for (unsigned i = 0; i < members.size(); ++i)
		{
			std::optional<std::string> elementStr = expressionToString(_expr.arguments.at(i), members[i]->type());
			elements.push_back(members[i]->name() + (elementStr.has_value() ?  ": " + elementStr.value() : ""));
		}
		return "{" + boost::algorithm::join(elements, ", ") + "}";
	}

	return {};
}

std::vector<std::optional<std::string>> formatExpressions(
	std::vector<smtutil::Expression> const& _exprs,
	std::vector<frontend::Type const*> const& _types
)
{
	solAssert(_exprs.size() == _types.size());
	std::vector<std::optional<std::string>> strExprs;
	for (unsigned i = 0; i < _exprs.size(); ++i)
		strExprs.push_back(expressionToString(_exprs.at(i), _types.at(i)));
	return strExprs;
}

}
