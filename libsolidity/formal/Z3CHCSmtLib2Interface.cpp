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

#include <libsolidity/formal/Z3CHCSmtLib2Interface.h>

#include <libsolidity/interface/UniversalCallback.h>

#include <libsmtutil/SMTLib2Parser.h>

#include <boost/algorithm/string/predicate.hpp>

#include <stack>

#ifdef EMSCRIPTEN_BUILD
#include <z3++.h>
#endif

using namespace solidity::frontend::smt;
using namespace solidity::smtutil;

Z3CHCSmtLib2Interface::Z3CHCSmtLib2Interface(
	frontend::ReadCallback::Callback _smtCallback,
	std::optional<unsigned int> _queryTimeout,
	bool _computeInvariants
): CHCSmtLib2Interface({}, std::move(_smtCallback), _queryTimeout), m_computeInvariants(_computeInvariants)
{
#ifdef EMSCRIPTEN_BUILD
	constexpr int resourceLimit = 2000000;
	if (m_queryTimeout)
		z3::set_param("timeout", int(*m_queryTimeout));
	else
		z3::set_param("rlimit", resourceLimit);
	z3::set_param("rewriter.pull_cheap_ite", true);
	z3::set_param("fp.spacer.q3.use_qgen", true);
	z3::set_param("fp.spacer.mbqi", false);
	z3::set_param("fp.spacer.ground_pobs", false);
#endif
}

void Z3CHCSmtLib2Interface::setupSmtCallback(bool _enablePreprocessing)
{
	if (auto* universalCallback = m_smtCallback.target<frontend::UniversalCallback>())
		universalCallback->smtCommand().setZ3(m_queryTimeout, _enablePreprocessing, m_computeInvariants);
}

CHCSolverInterface::QueryResult Z3CHCSmtLib2Interface::query(smtutil::Expression const& _block)
{
	setupSmtCallback(true);
	std::string query = dumpQuery(_block);
#ifdef EMSCRIPTEN_BUILD
	z3::set_param("fp.xform.slice", true);
	z3::set_param("fp.xform.inline_linear", true);
	z3::set_param("fp.xform.inline_eager", true);
	std::string response = [&](){ z3::context context; return Z3_eval_smtlib2_string(context, query.c_str()); }();
#else
	std::string response = querySolver(query);
#endif
	// NOTE: Our internal semantics is UNSAT -> SAFE and SAT -> UNSAFE, which corresponds to usual SMT-based model checking
	// However, with CHC solvers, the meaning is flipped, UNSAT -> UNSAFE and SAT -> SAFE.
	// So we have to flip the answer.
	if (boost::starts_with(response, "unsat"))
	{
		// Repeat the query with preprocessing disabled, to get the full proof
		setupSmtCallback(false);
		query = "(set-option :produce-proofs true)" + query + "\n(get-proof)";
#ifdef EMSCRIPTEN_BUILD
		z3::context context;
		z3::set_param("fp.xform.slice", false);
		z3::set_param("fp.xform.inline_linear", false);
		z3::set_param("fp.xform.inline_eager", false);
		response = Z3_eval_smtlib2_string(context, query.c_str());
#else
		response = querySolver(query);
#endif
		setupSmtCallback(true);
		if (!boost::starts_with(response, "unsat"))
			return {CheckResult::SATISFIABLE, Expression(true), {}};
		return {CheckResult::SATISFIABLE, Expression(true), graphFromZ3Answer(response)};
	}

	CheckResult result;
	if (boost::starts_with(response, "sat"))
	{
		auto maybeInvariants = invariantsFromSolverResponse(response);
		return {CheckResult::UNSATISFIABLE, maybeInvariants.value_or(Expression(true)), {}};
	}
	else if (boost::starts_with(response, "unknown"))
		result = CheckResult::UNKNOWN;
	else
		result = CheckResult::ERROR;

	return {result, Expression(true), {}};
}


CHCSolverInterface::CexGraph Z3CHCSmtLib2Interface::graphFromZ3Answer(std::string const& _proof) const
{
	std::stringstream ss(_proof);
	std::string answer;
	ss >> answer;
	smtAssert(answer == "unsat");

	SMTLib2Parser parser(ss);
	if (parser.isEOF()) // No proof from Z3
		return {};
	// For some reason Z3 outputs everything as a single s-expression
	SMTLib2Expression parsedOutput;
	try
	{
		parsedOutput = parser.parseExpression();
	}
	catch (SMTLib2Parser::ParsingException&)
	{
		return {};
	}
	solAssert(parser.isEOF());
	solAssert(!isAtom(parsedOutput));
	auto& commands = asSubExpressions(parsedOutput);
	ScopedParser expressionParser(m_context);
	for (auto& command: commands)
	{
		if (isAtom(command))
			continue;

		auto const& args = asSubExpressions(command);
		solAssert(args.size() > 0);
		auto const& head = args[0];
		if (!isAtom(head))
			continue;

		// Z3 can introduce new helper predicates to be used in the proof
		// e.g., "(declare-fun query!0 (Bool Bool Bool Int Int Bool Bool Bool Bool Bool Bool Bool Int) Bool)"
		if (asAtom(head) == "declare-fun")
		{
			solAssert(args.size() == 4);
			auto const& name = args[1];
			auto const& domainSorts = args[2];
			auto const& codomainSort = args[3];
			solAssert(isAtom(name));
			solAssert(!isAtom(domainSorts));
			expressionParser.addVariableDeclaration(asAtom(name), expressionParser.toSort(codomainSort));
		}
		// The subexpression starting with "proof" contains the whole proof, which we need to transform to our internal
		// representation
		else if (asAtom(head) == "proof")
		{
			inlineLetExpressions(command);
			return graphFromSMTLib2Expression(command, expressionParser);
		}
	}
	return {};
}

CHCSolverInterface::CexGraph Z3CHCSmtLib2Interface::graphFromSMTLib2Expression(
	SMTLib2Expression const& _proof,
	ScopedParser& _context
)
{
	auto fact = [](SMTLib2Expression const& _node) -> SMTLib2Expression const& {
		if (isAtom(_node))
			return _node;
		smtAssert(!asSubExpressions(_node).empty());
		return asSubExpressions(_node).back();
	};
	smtAssert(!isAtom(_proof));
	auto const& proofArgs = asSubExpressions(_proof);
	smtAssert(proofArgs.size() == 2);
	smtAssert(isAtom(proofArgs.at(0)) && asAtom(proofArgs.at(0)) == "proof");
	auto const& proofNode = proofArgs.at(1);
	auto const& derivedFact = fact(proofNode);
	if (isAtom(proofNode) || !isAtom(derivedFact) || asAtom(derivedFact) != "false")
		return {};

	CHCSolverInterface::CexGraph graph;

	std::stack<SMTLib2Expression const*> proofStack;
	proofStack.push(&asSubExpressions(proofNode).at(1));

	std::map<SMTLib2Expression const*, unsigned> visitedIds;
	unsigned nextId = 0;

	auto const* root = proofStack.top();
	auto const& derivedRootFact = fact(*root);
	visitedIds.emplace(root, nextId++);
	graph.nodes.emplace(visitedIds.at(root), _context.toSMTUtilExpression(derivedRootFact));

	auto isHyperRes = [](SMTLib2Expression const& expr) {
		if (isAtom(expr)) return false;
		auto const& subExprs = asSubExpressions(expr);
		smtAssert(!subExprs.empty());
		auto const& op = subExprs.at(0);
		if (isAtom(op)) return false;
		auto const& opExprs = asSubExpressions(op);
		if (opExprs.size() < 2) return false;
		auto const& ruleName = opExprs.at(1);
		return isAtom(ruleName) && asAtom(ruleName) == "hyper-res";
	};

	while (!proofStack.empty())
	{
		auto const* currentNode = proofStack.top();
		smtAssert(visitedIds.find(currentNode) != visitedIds.end());
		auto id = visitedIds.at(currentNode);
		smtAssert(graph.nodes.count(id));
		proofStack.pop();

		if (isHyperRes(*currentNode))
		{
			auto const& args = asSubExpressions(*currentNode);
			smtAssert(args.size() > 1);
			// args[0] is the name of the rule
			// args[1] is the clause used
			// last argument is the derived fact
			// the arguments in the middle are the facts where we need to recurse
			for (unsigned i = 2; i < args.size() - 1; ++i)
			{
				auto const* child = &args[i];
				if (!visitedIds.count(child))
				{
					visitedIds.emplace(child, nextId++);
					proofStack.push(child);
				}

				auto childId = visitedIds.at(child);
				if (!graph.nodes.count(childId))
				{
					graph.nodes.emplace(childId, _context.toSMTUtilExpression(fact(*child)));
					graph.edges[childId] = {};
				}

				graph.edges[id].push_back(childId);
			}
		}
	}
	return graph;
}

