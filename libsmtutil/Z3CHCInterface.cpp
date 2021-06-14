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

#include <libsmtutil/Z3CHCInterface.h>

#include <libsolutil/CommonIO.h>

#include <set>
#include <stack>

using namespace std;
using namespace solidity;
using namespace solidity::smtutil;

Z3CHCInterface::Z3CHCInterface(optional<unsigned> _queryTimeout):
	CHCSolverInterface(_queryTimeout),
	m_z3Interface(make_unique<Z3Interface>(m_queryTimeout)),
	m_context(m_z3Interface->context()),
	m_solver(*m_context)
{
	Z3_get_version(
		&get<0>(m_version),
		&get<1>(m_version),
		&get<2>(m_version),
		&get<3>(m_version)
	);

	// These need to be set globally.
	z3::set_param("rewriter.pull_cheap_ite", true);

	if (m_queryTimeout)
		m_context->set("timeout", int(*m_queryTimeout));
	else
		z3::set_param("rlimit", Z3Interface::resourceLimit);

	setSpacerOptions();
}

void Z3CHCInterface::declareVariable(string const& _name, SortPointer const& _sort)
{
	smtAssert(_sort, "");
	m_z3Interface->declareVariable(_name, _sort);
}

void Z3CHCInterface::registerRelation(Expression const& _expr)
{
	m_solver.register_relation(m_z3Interface->functions().at(_expr.name));
}

void Z3CHCInterface::addRule(Expression const& _expr, string const& _name)
{
	z3::expr rule = m_z3Interface->toZ3Expr(_expr);
	if (m_z3Interface->constants().empty())
		m_solver.add_rule(rule, m_context->str_symbol(_name.c_str()));
	else
	{
		z3::expr_vector variables(*m_context);
		for (auto const& var: m_z3Interface->constants())
			variables.push_back(var.second);
		z3::expr boundRule = z3::forall(variables, rule);
		m_solver.add_rule(boundRule, m_context->str_symbol(_name.c_str()));
	}
}

tuple<CheckResult, Expression, CHCSolverInterface::CexGraph> Z3CHCInterface::query(Expression const& _expr)
{
	CheckResult result;
	try
	{
		z3::expr z3Expr = m_z3Interface->toZ3Expr(_expr);
		switch (m_solver.query(z3Expr))
		{
		case z3::check_result::sat:
		{
			result = CheckResult::SATISFIABLE;
			// z3 version 4.8.8 modified Spacer to also return
			// proofs containing nonlinear clauses.
			if (m_version >= tuple(4, 8, 8, 0))
			{
				auto proof = m_solver.get_answer();
				return {result, Expression(true), cexGraph(proof)};
			}
			break;
		}
		case z3::check_result::unsat:
		{
			result = CheckResult::UNSATISFIABLE;
			auto invariants = m_z3Interface->fromZ3Expr(m_solver.get_answer());
			return {result, move(invariants), {}};
		}
		case z3::check_result::unknown:
		{
			result = CheckResult::UNKNOWN;
			break;
		}
		}
		// TODO retrieve model / invariants
	}
	catch (z3::exception const& _err)
	{
		set<string> msgs{
			/// Resource limit (rlimit) exhausted.
			"max. resource limit exceeded",
			/// User given timeout exhausted.
			"canceled"
		};
		if (msgs.count(_err.msg()))
			result = CheckResult::UNKNOWN;
		else
			result = CheckResult::ERROR;
	}

	return {result, Expression(true), {}};
}

void Z3CHCInterface::setSpacerOptions(bool _preProcessing)
{
	// Spacer options.
	// These needs to be set in the solver.
	// https://github.com/Z3Prover/z3/blob/master/src/muz/base/fp_params.pyg
	z3::params p(*m_context);
	// These are useful for solving problems with arrays and loops.
	// Use quantified lemma generalizer.
	p.set("fp.spacer.q3.use_qgen", true);
	p.set("fp.spacer.mbqi", false);
	// Ground pobs by using values from a model.
	p.set("fp.spacer.ground_pobs", false);

	// Spacer optimization should be
	// - enabled for better solving (default)
	// - disable for counterexample generation
	p.set("fp.xform.slice", _preProcessing);
	p.set("fp.xform.inline_linear", _preProcessing);
	p.set("fp.xform.inline_eager", _preProcessing);

	m_solver.set(p);
}

/**
Convert a ground refutation into a linear or nonlinear counterexample.
The counterexample is given as an implication graph of the form
`premises => conclusion` where `premises` are the predicates
from the body of nonlinear clauses, representing the proof graph.

This function is based on and similar to
https://github.com/Z3Prover/z3/blob/z3-4.8.8/src/muz/spacer/spacer_context.cpp#L2919
(spacer::context::get_ground_sat_answer)
which generates linear counterexamples.
It is modified here to accept nonlinear CHCs as well, generating a DAG
instead of a path.
*/
CHCSolverInterface::CexGraph Z3CHCInterface::cexGraph(z3::expr const& _proof)
{
	/// The root fact of the refutation proof is `false`.
	/// The node itself is not a hyper resolution, so we need to
	/// extract the `query` hyper resolution node from the
	/// `false` node (the first child).
	/// The proof has the shape above for z3 >=4.8.8.
	/// If an older version is used, this check will fail and no
	/// counterexample will be generated.
	if (!_proof.is_app() || fact(_proof).decl().decl_kind() != Z3_OP_FALSE)
		return {};

	CexGraph graph;

	stack<z3::expr> proofStack;
	proofStack.push(_proof.arg(0));

	auto const& root = proofStack.top();
	graph.nodes.emplace(root.id(), m_z3Interface->fromZ3Expr(fact(root)));

	set<unsigned> visited;
	visited.insert(root.id());

	while (!proofStack.empty())
	{
		z3::expr proofNode = proofStack.top();
		smtAssert(graph.nodes.count(proofNode.id()), "");
		proofStack.pop();

		if (proofNode.is_app() && proofNode.decl().decl_kind() == Z3_OP_PR_HYPER_RESOLVE)
		{
			smtAssert(proofNode.num_args() > 0, "");
			for (unsigned i = 1; i < proofNode.num_args() - 1; ++i)
			{
				z3::expr child = proofNode.arg(i);
				if (!visited.count(child.id()))
				{
					visited.insert(child.id());
					proofStack.push(child);
				}

				if (!graph.nodes.count(child.id()))
				{
					graph.nodes.emplace(child.id(), m_z3Interface->fromZ3Expr(fact(child)));
					graph.edges[child.id()] = {};
				}

				graph.edges[proofNode.id()].push_back(child.id());
			}
		}
	}

	return graph;
}

z3::expr Z3CHCInterface::fact(z3::expr const& _node)
{
	smtAssert(_node.is_app(), "");
	if (_node.num_args() == 0)
		return _node;
	return _node.arg(_node.num_args() - 1);
}

string Z3CHCInterface::name(z3::expr const& _predicate)
{
	smtAssert(_predicate.is_app(), "");
	return _predicate.decl().name().str();
}

vector<string> Z3CHCInterface::arguments(z3::expr const& _predicate)
{
	smtAssert(_predicate.is_app(), "");
	vector<string> args;
	for (unsigned i = 0; i < _predicate.num_args(); ++i)
		args.emplace_back(_predicate.arg(i).to_string());
	return args;
}
