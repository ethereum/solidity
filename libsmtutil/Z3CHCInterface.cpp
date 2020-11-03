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

namespace
{
class SpacerStrategy
{
public:
	virtual pair<CheckResult, optional<z3::expr>> runQuery(z3::expr _expr) = 0;
	virtual ~SpacerStrategy() = default;
};

class CustomizableSpacerStrategy : public SpacerStrategy
{
protected:
	z3::fixedpoint& m_solver;
	z3::params m_params;
public:
	explicit CustomizableSpacerStrategy(z3::fixedpoint& _solver): m_solver(_solver), m_params(_solver.ctx()) {}

	virtual ~CustomizableSpacerStrategy() = default;

	pair<CheckResult, optional<z3::expr>> runQuery(z3::expr expr) override
	{
		setParamsForSolver();
		try
		{
			switch (m_solver.query(expr))
			{
				case z3::check_result::sat:
					return { CheckResult::SATISFIABLE, m_solver.get_answer() };
				case z3::check_result::unsat:
					return { CheckResult::UNSATISFIABLE, {} };
				case z3::check_result::unknown:
					return { CheckResult::UNKNOWN, {} };
			}
		}
		catch (z3::exception const & _err)
		{
			set<string> msgs{
				/// Resource limit (rlimit) exhausted.
				"max. resource limit exceeded",
				/// User given timeout exhausted.
				"canceled"
			};
			auto res = msgs.count(_err.msg()) ? CheckResult::UNKNOWN : CheckResult::ERROR;
			return { res, {} };
		}
		smtAssert(false, "Unreachable!"); // GCC complains if this is missing.
	}

	virtual void setParamsForSolver()
	{
		m_solver.set(m_params);
	}

	template<typename T>
	void setOptions(T const& optionMap)
	{
		for (auto const& entry: optionMap)
			m_params.set(entry.first, entry.second);
	}
};

class DefaultSpacerStrategy : public CustomizableSpacerStrategy
{
public:
	explicit DefaultSpacerStrategy(z3::fixedpoint& _solver) : CustomizableSpacerStrategy(_solver)
	{
		// These needs to be set in the solver.
		// https://github.com/Z3Prover/z3/blob/master/src/muz/base/fp_params.pyg
		auto options = map<char const*, bool>
		{
			// These are useful for solving problems with arrays and loops.
			// Use quantified lemma generalizer.
			{"fp.spacer.q3.use_qgen", true},
			{"fp.spacer.mbqi", false},
			// Ground pobs by using values from a model.
			{"fp.spacer.ground_pobs", false},

			{"fp.spacer.weak_abs", true},
			// preprocessing enabled
			{"fp.xform.slice", true},
			{"fp.xform.inline_linear", true},
			{"fp.xform.inline_eager", true}
		};
		setOptions(options);
	}

	virtual ~DefaultSpacerStrategy() = default;
};

class DefaultWithoutWeakAbstractionSpacerStrategy : public DefaultSpacerStrategy
{
public:
	explicit DefaultWithoutWeakAbstractionSpacerStrategy(z3::fixedpoint& _solver) : DefaultSpacerStrategy(_solver)
	{
		m_params.set("fp.spacer.weak_abs", false);
	}
};

//class InverseDefaultSpacerStrategy : public CustomizableSpacerStrategy
//{
//public:
//	explicit InverseDefaultSpacerStrategy(z3::fixedpoint& _solver) : CustomizableSpacerStrategy(_solver)
//	{
//		// These needs to be set in the solver.
//		// https://github.com/Z3Prover/z3/blob/master/src/muz/base/fp_params.pyg
//		setOptions(map<char const*, bool>{
//			// These are useful for solving problems with arrays and loops.
//			// Use quantified lemma generalizer.
//			{"fp.spacer.q3.use_qgen", false},
//			{"fp.spacer.mbqi", true},
//			// Ground pobs by using values from a model.
//			{"fp.spacer.ground_pobs", true},
//			// preprocessing enabled
//			{"fp.xform.slice", true},
//			{"fp.xform.inline_linear", true},
//			{"fp.xform.inline_eager", true}
//		});
//	}
//};


class SpacerStrategyPreprocessingDisabled : public SpacerStrategy
{
	unique_ptr<CustomizableSpacerStrategy> m_base;
public:
	explicit SpacerStrategyPreprocessingDisabled(unique_ptr<CustomizableSpacerStrategy> _base) : m_base(move(_base))
	{
		m_base->setOptions(map<char const*, bool>
		{
			{"fp.xform.slice", false},
			{"fp.xform.inline_linear", false},
			{"fp.xform.inline_eager", false}
		});
	}

	pair<CheckResult, optional<z3::expr>> runQuery(z3::expr expr) override
	{
		return m_base->runQuery(expr);
	}
};

class EnsureCorrectCounterExampleStrategy : public SpacerStrategy
{
	unique_ptr<CustomizableSpacerStrategy> m_base;
public:
	explicit EnsureCorrectCounterExampleStrategy(unique_ptr<CustomizableSpacerStrategy> _base) : m_base(move(_base)) {}

	pair<CheckResult, optional<z3::expr>> runQuery(z3::expr _expr) override
	{
		auto [result, proof] = m_base->runQuery(_expr);
		if (result == CheckResult::SATISFIABLE)
		{
			// Even though the problem is SAT, Spacer's pre processing makes counterexamples incomplete.
			// We now disable those optimizations and check whether we can still solve the problem.
			auto [resultNoOpt, proofNoOpt] = SpacerStrategyPreprocessingDisabled(make_unique<CustomizableSpacerStrategy>(*m_base)).runQuery(_expr);
			if (resultNoOpt == CheckResult::SATISFIABLE)
				proof = proofNoOpt;
		}
		return {result, proof};
	}
};

class ComposedSpacerStrategy : public SpacerStrategy
{
	unique_ptr<SpacerStrategy> m_main;
	unique_ptr<SpacerStrategy> m_backup;

public:
	ComposedSpacerStrategy(unique_ptr<SpacerStrategy> _main, unique_ptr<SpacerStrategy> _backup)
		: m_main(move(_main)), m_backup(move(_backup)) {}

	pair<CheckResult, optional<z3::expr>> runQuery(z3::expr _expr) override
	{
		auto res = m_main->runQuery(_expr);
		if (res.first == CheckResult::SATISFIABLE || res.first == CheckResult::UNSATISFIABLE)
			return res;
		return m_backup->runQuery(_expr);
	}
};
}

Z3CHCInterface::Z3CHCInterface(optional<unsigned> _queryTimeout):
	CHCSolverInterface(_queryTimeout),
	m_z3Interface(make_unique<Z3Interface>(m_queryTimeout)),
	m_context(m_z3Interface->context()),
	m_solver(*m_context)
{
	// These need to be set globally.
	z3::set_param("rewriter.pull_cheap_ite", true);

	if (m_queryTimeout)
		m_context->set("timeout", int(*m_queryTimeout));
	else
		z3::set_param("rlimit", Z3Interface::resourceLimit);
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

pair<CheckResult, CHCSolverInterface::CexGraph> Z3CHCInterface::query(Expression const& _expr)
{
	CHCSolverInterface::CexGraph cex;
	z3::expr z3Expr = m_z3Interface->toZ3Expr(_expr);

	auto strategy = make_unique<ComposedSpacerStrategy>(
		make_unique<EnsureCorrectCounterExampleStrategy>(make_unique<DefaultSpacerStrategy>(m_solver)),
		make_unique<EnsureCorrectCounterExampleStrategy>(make_unique<DefaultWithoutWeakAbstractionSpacerStrategy>(m_solver))
	);
	auto [result, proof] = strategy->runQuery(z3Expr);
	switch (result) {
		case CheckResult::SATISFIABLE: {
			if (proof)
				cex = cexGraph(proof.value());
			break;
		}
		case CheckResult::UNSATISFIABLE: {
			// TODO retrieve invariants.
			break;
		}
		case CheckResult::UNKNOWN:
		case CheckResult::ERROR:
		{
			break;
		}
		default:
			smtAssert(false, "");
	}
	// TODO retrieve model / invariants
	return {result, cex};
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
CHCSolverInterface::CexGraph Z3CHCInterface::cexGraph(z3::expr const& _proof) const
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

z3::expr Z3CHCInterface::fact(z3::expr const& _node) const
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
