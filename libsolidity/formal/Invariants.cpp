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

#include <libsolidity/formal/Invariants.h>

#include <libsolidity/formal/SMTEncoder.h>

#include <libsolutil/Algorithms.h>
#include <libsolutil/CommonData.h>

#include <boost/algorithm/string.hpp>

#include <range/v3/algorithm/for_each.hpp>

using namespace std;
using boost::algorithm::starts_with;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::smtutil;
using namespace solidity::frontend::smt;

namespace solidity::frontend::smt
{

namespace
{

// Infix operators with format replacements.
static map<string, string> const infixOps{
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
static set<string> const arrayOps{"select", "store", "const_array"};
static set<string> const ufs{"keccak256", "sha256", "ripemd160", "ecrecover"};

string formatDatatypeAccessor(smtutil::Expression const& _expr, vector<string> const& _args)
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

	string a = "accessor_";
	// Struct members have suffix "accessor_<memberName>".
	string type = op.substr(op.rfind(a) + a.size());
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

string formatGenericOp(smtutil::Expression const& _expr, vector<string> const& _args)
{
	return _expr.name + "(" + boost::algorithm::join(_args, ", ") + ")";
}

string formatInfixOp(string const& _op, vector<string> const& _args)
{
	return "(" + boost::algorithm::join(_args, " " + _op + " ") + ")";
}

string formatArrayOp(smtutil::Expression const& _expr, vector<string> const& _args)
{
	if (_expr.name == "select")
	{
		auto const& a0 = _args.at(0);
		if (ufs.count(a0) || starts_with(a0, "t_function_abi"))
			return _args.at(0) + "(" + _args.at(1) + ")";
		return _args.at(0) + "[" + _args.at(1) + "]";
	}
	if (_expr.name == "store")
		return "(" + _args.at(0) + "[" + _args.at(1) + "] := " + _args.at(2) + ")";
	return formatGenericOp(_expr, _args);
}

string formatUnaryOp(smtutil::Expression const& _expr, vector<string> const& _args)
{
	if (_expr.name == "not")
		return "!" + _args.at(0);
	// Other operators such as exists may end up here.
	return formatGenericOp(_expr, _args);
}

smtutil::Expression substitute(smtutil::Expression _from, map<string, string> const& _subst)
{
	// TODO For now we ignore nested quantifier expressions,
	// but should support them in the future.
	if (_from.name == "forall" || _from.name == "exists")
		return smtutil::Expression(true);
	if (_subst.count(_from.name))
		_from.name = _subst.at(_from.name);
	for (auto& arg: _from.arguments)
		arg = substitute(arg, _subst);
	return _from;
}

string toSolidityStr(smtutil::Expression const& _expr)
{
	auto const& op = _expr.name;

	auto const& args = _expr.arguments;
	auto strArgs = applyMap(args, [](auto const& _arg) { return toSolidityStr(_arg); });

	// Constant or variable.
	if (args.empty())
		return op;

	if (starts_with(op, "dt_accessor"))
		return formatDatatypeAccessor(_expr, strArgs);

	// Some of these (and, or, +, *) may have >= 2 arguments from z3.
	if (infixOps.count(op))
		return formatInfixOp(infixOps.at(op), strArgs);

	if (arrayOps.count(op))
		return formatArrayOp(_expr, strArgs);

	if (args.size() == 1)
		return formatUnaryOp(_expr, strArgs);

	// Other operators such as bv2int, int2bv may end up here.
	return op + "(" + boost::algorithm::join(strArgs, ", ") + ")";
}

}

map<ASTNode const*, set<string>> collectInvariants(smtutil::Expression const& _proof, set<Predicate const*> const& _predicates)
{
	set<string> targets{"interface_", "nondet_interface_"};
	map<string, pair<smtutil::Expression, smtutil::Expression>> eqs;
	// Collect equalities where one of the sides is a predicate we're interested in.
	BreadthFirstSearch<smtutil::Expression const*>{{&_proof}}.run([&](auto&& _expr, auto&& _addChild) {
		if (_expr->name == "=")
			for (auto const& t: targets)
			{
				auto arg0 = _expr->arguments.at(0);
				auto arg1 = _expr->arguments.at(1);
				if (starts_with(arg0.name, t))
					eqs.insert({arg0.name, {arg0, move(arg1)}});
				else if (starts_with(arg1.name, t))
					eqs.insert({arg1.name, {arg1, move(arg0)}});
			}
		for (auto const& arg: _expr->arguments)
			_addChild(&arg);
	});

	map<ASTNode const*, set<string>> invariants;
	for (auto pred: _predicates)
	{
		auto predName = pred->functor().name;
		if (!eqs.count(predName))
			continue;

		solAssert(pred->contextContract(), "");
		auto const& stateVars = SMTEncoder::stateVariablesIncludingInheritedAndPrivate(*pred->contextContract());

		map<string, string> subst;
		auto const& [predExpr, invExpr] = eqs.at(predName);
		size_t nArgs = predExpr.arguments.size();
		// TODO these blocks are ugly, fix them.
		if (pred->isInterface())
		{
			solAssert(starts_with(predName, "interface"), "");
			subst[predExpr.arguments.at(0).name] = "address(this)";
			solAssert(nArgs == stateVars.size() + 4, "");
			for (size_t i = nArgs - stateVars.size(); i < nArgs; ++i)
				subst[predExpr.arguments.at(i).name] = stateVars.at(i - 4)->name();
		}
		else if (pred->isNondetInterface())
		{
			solAssert(starts_with(predName, "nondet_interface"), "");
			subst[predExpr.arguments.at(0).name] = "<errorCode>";
			subst[predExpr.arguments.at(1).name] = "address(this)";
			solAssert(nArgs == stateVars.size() * 2 + 6, "");
			for (size_t i = nArgs - stateVars.size(), s = 0; i < nArgs; ++i, ++s)
				subst[predExpr.arguments.at(i).name] = stateVars.at(s)->name() + "'";
			for (size_t i = nArgs - (stateVars.size() * 2 + 1), s = 0; i < nArgs - (stateVars.size() + 1); ++i, ++s)
				subst[predExpr.arguments.at(i).name] = stateVars.at(s)->name();
		}

		static set<string> const ignore{"true", "false"};
		auto r = substitute(invExpr, subst);
		// No point in reporting true/false as invariants.
		if (!ignore.count(r.name))
			invariants[pred->programNode()].insert(toSolidityStr(r));
	}
	return invariants;
}

}
