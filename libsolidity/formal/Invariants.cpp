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

#include <libsolidity/formal/ExpressionFormatter.h>
#include <libsolidity/formal/SMTEncoder.h>

#include <libsolutil/Algorithms.h>

#include <boost/algorithm/string.hpp>

using namespace std;
using boost::algorithm::starts_with;
using namespace solidity;
using namespace solidity::smtutil;
using namespace solidity::frontend::smt;

namespace solidity::frontend::smt
{

map<Predicate const*, set<string>> collectInvariants(
	smtutil::Expression const& _proof,
	set<Predicate const*> const& _predicates,
	ModelCheckerInvariants const& _invariantsSetting
)
{
	set<string> targets;
	if (_invariantsSetting.has(InvariantType::Contract))
		targets.insert("interface_");
	if (_invariantsSetting.has(InvariantType::Reentrancy))
		targets.insert("nondet_interface_");

	map<string, pair<smtutil::Expression, smtutil::Expression>> equalities;
	// Collect equalities where one of the sides is a predicate we're interested in.
	util::BreadthFirstSearch<smtutil::Expression const*>{{&_proof}}.run([&](auto&& _expr, auto&& _addChild) {
		if (_expr->name == "=")
			for (auto const& t: targets)
			{
				auto arg0 = _expr->arguments.at(0);
				auto arg1 = _expr->arguments.at(1);
				if (starts_with(arg0.name, t))
					equalities.insert({arg0.name, {arg0, move(arg1)}});
				else if (starts_with(arg1.name, t))
					equalities.insert({arg1.name, {arg1, move(arg0)}});
			}
		for (auto const& arg: _expr->arguments)
			_addChild(&arg);
	});

	map<Predicate const*, set<string>> invariants;
	for (auto pred: _predicates)
	{
		auto predName = pred->functor().name;
		if (!equalities.count(predName))
			continue;

		solAssert(pred->contextContract(), "");

		auto const& [predExpr, invExpr] = equalities.at(predName);

		static set<string> const ignore{"true", "false"};
		auto r = substitute(invExpr, pred->expressionSubstitution(predExpr));
		// No point in reporting true/false as invariants.
		if (!ignore.count(r.name))
			invariants[pred].insert(toSolidityStr(r));
	}
	return invariants;
}

}
