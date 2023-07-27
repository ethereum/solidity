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

#include <libsmtutil/CHCSmtLib2Interface.h>

#include <libsolutil/Keccak256.h>
#include <libsolutil/StringUtils.h>
#include <libsolutil/Visitor.h>

#include <liblangutil/Common.h>


#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <range/v3/view.hpp>
#include <range/v3/algorithm/find.hpp>
#include <range/v3/algorithm/find_if.hpp>

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <stdexcept>
#include <unordered_map>
#include <variant>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend;
using namespace solidity::langutil;
using namespace solidity::smtutil;

CHCSmtLib2Interface::CHCSmtLib2Interface(
	std::map<h256, std::string> const& _queryResponses,
	ReadCallback::Callback _smtCallback,
	SMTSolverChoice _enabledSolvers,
	std::optional<unsigned> _queryTimeout
):
	CHCSolverInterface(_queryTimeout),
	m_smtlib2(std::make_unique<SMTLib2Interface>(_queryResponses, _smtCallback, _enabledSolvers,  m_queryTimeout)),
	m_queryResponses(std::move(_queryResponses)),
	m_smtCallback(_smtCallback),
	m_enabledSolvers(_enabledSolvers)
{
	reset();
}

void CHCSmtLib2Interface::reset()
{
	m_accumulatedOutput.clear();
	m_variables.clear();
	m_unhandledQueries.clear();
}

void CHCSmtLib2Interface::registerRelation(Expression const& _expr)
{
	smtAssert(_expr.sort);
	smtAssert(_expr.sort->kind == Kind::Function);
	if (!m_variables.count(_expr.name))
	{
		auto fSort = std::dynamic_pointer_cast<FunctionSort>(_expr.sort);
		std::string domain = toSmtLibSort(fSort->domain);
		// Relations are predicates which have implicit codomain Bool.
		m_variables.insert(_expr.name);
		write(
			"(declare-fun |" +
			_expr.name +
			"| " +
			domain +
			" Bool)"
		);
	}
}

void CHCSmtLib2Interface::addRule(Expression const& _expr, std::string const& /*_name*/)
{
	write(
		"(assert\n(forall " + forall() + "\n" +
		m_smtlib2->toSExpr(_expr) +
		"))\n\n"
	);
}

std::tuple<CheckResult, Expression, CHCSolverInterface::CexGraph> CHCSmtLib2Interface::query(Expression const& _block)
{
	std::string query = dumpQuery(_block);
	std::string response = querySolver(query);

	CheckResult result;
	// TODO proper parsing
	if (boost::starts_with(response, "sat"))
		result = CheckResult::UNSATISFIABLE;
	else if (boost::starts_with(response, "unsat"))
	{
		result = CheckResult::SATISFIABLE;
		return {result, Expression(true), graphFromZ3Proof(response)};
	}
	else if (boost::starts_with(response, "unknown"))
		result = CheckResult::UNKNOWN;
	else
		result = CheckResult::ERROR;

	return {result, Expression(true), {}};
}

void CHCSmtLib2Interface::declareVariable(std::string const& _name, SortPointer const& _sort)
{
	smtAssert(_sort);
	if (_sort->kind == Kind::Function)
		declareFunction(_name, _sort);
	else if (!m_variables.count(_name))
	{
		m_variables.insert(_name);
		write("(declare-var |" + _name + "| " + toSmtLibSort(*_sort) + ')');
	}
}

std::string CHCSmtLib2Interface::toSmtLibSort(Sort const& _sort)
{
	return m_smtlib2->toSmtLibSort(_sort);
}

std::string CHCSmtLib2Interface::toSmtLibSort(std::vector<SortPointer> const& _sorts)
{
	return m_smtlib2->toSmtLibSort(_sorts);
}

std::string CHCSmtLib2Interface::forall()
{
	std::string vars("(");
	for (auto const& [name, sort]: m_smtlib2->variables())
	{
		solAssert(sort, "");
		if (sort->kind != Kind::Function)
			vars += " (" + name + " " + toSmtLibSort(*sort) + ")";
	}
	vars += ")";
	return vars;
}

void CHCSmtLib2Interface::declareFunction(std::string const& _name, SortPointer const& _sort)
{
	smtAssert(_sort);
	smtAssert(_sort->kind == Kind::Function);
	// TODO Use domain and codomain as key as well
	if (!m_variables.count(_name))
	{
		auto fSort = std::dynamic_pointer_cast<FunctionSort>(_sort);
		smtAssert(fSort->codomain);
		std::string domain = toSmtLibSort(fSort->domain);
		std::string codomain = toSmtLibSort(*fSort->codomain);
		m_variables.insert(_name);
		write(
			"(declare-fun |" +
			_name +
			"| " +
			domain +
			" " +
			codomain +
			")"
		);
	}
}

void CHCSmtLib2Interface::write(std::string _data)
{
	m_accumulatedOutput += std::move(_data) + "\n";
}

std::string CHCSmtLib2Interface::querySolver(std::string const& _input)
{
	util::h256 inputHash = util::keccak256(_input);
	if (m_queryResponses.count(inputHash))
		return m_queryResponses.at(inputHash);

	smtAssert(m_enabledSolvers.eld || m_enabledSolvers.z3);
	smtAssert(m_smtCallback, "Callback must be set!");
	std::string solverBinary = [&](){
		if (m_enabledSolvers.eld)
			return "eld";
		if (m_enabledSolvers.z3)
			return "z3 rlimit=1000000 fp.spacer.q3.use_qgen=true fp.spacer.mbqi=false fp.spacer.ground_pobs=false";
		return "";
	}();
	auto result = m_smtCallback(ReadCallback::kindString(ReadCallback::Kind::SMTQuery) + " " + solverBinary, _input);
	if (result.success)
	{
		if (m_enabledSolvers.z3 and boost::starts_with(result.responseOrErrorMessage, "unsat"))
		{
			solverBinary += " fp.xform.slice=false fp.xform.inline_linear=false fp.xform.inline_eager=false";
			std::string extendedQuery = "(set-option :produce-proofs true)" + _input + "\n(get-proof)";
			auto secondResult = m_smtCallback(ReadCallback::kindString(ReadCallback::Kind::SMTQuery) + " " + solverBinary, extendedQuery);
			if (secondResult.success)
				return secondResult.responseOrErrorMessage;
		}
		return result.responseOrErrorMessage;
	}

	m_unhandledQueries.push_back(_input);
	return "unknown\n";
}

std::string CHCSmtLib2Interface::dumpQuery(Expression const& _expr)
{
	std::stringstream s;

	s
		<< createHeaderAndDeclarations()
		<< m_accumulatedOutput << std::endl
		<< createQueryAssertion(_expr.name) << std::endl
		<< "(check-sat)" << std::endl;

	return s.str();
}

std::string CHCSmtLib2Interface::createHeaderAndDeclarations() {
	std::stringstream s;
	if (m_queryTimeout)
		s << "(set-option :timeout " + std::to_string(*m_queryTimeout) + ")\n";
	s << "(set-logic HORN)" << std::endl;

	for (auto const& decl: m_smtlib2->userSorts() | ranges::views::values)
		s << decl << std::endl;

	return s.str();
}

std::string CHCSmtLib2Interface::createQueryAssertion(std::string name) {
	return "(assert\n(forall " + forall() + "\n" +	"(=> " + name + " false)))";
}

std::string CHCSmtLib2Interface::SMTLib2Expression::toString() const
{
	return std::visit(GenericVisitor{
			[](std::string const& _sv) { return _sv; },
			[](std::vector<SMTLib2Expression> const& _subExpr) {
				std::vector<std::string> formatted;
				for (auto const& item: _subExpr)
					formatted.emplace_back(item.toString());
				return "(" + joinHumanReadable(formatted, " ") + ")";
			}
	}, data);
}

namespace
{
	using SMTLib2Expression = CHCSmtLib2Interface::SMTLib2Expression;
	bool isNumber(std::string const& _expr)
	{
		for (char c: _expr)
			if (!isDigit(c) && c != '.')
				return false;
		return true;
	}

	bool isAtom(SMTLib2Expression const & expr)
	{
		return std::holds_alternative<std::string>(expr.data);
	}

	std::string const& asAtom(SMTLib2Expression const& expr)
	{
		assert(isAtom(expr));
		return std::get<std::string>(expr.data);
	}

	auto const& asSubExpressions(SMTLib2Expression const& expr)
	{
		assert(!isAtom(expr));
		return std::get<SMTLib2Expression::args_t>(expr.data);
	}

	class SMTLibTranslationContext
	{
		SMTLib2Interface const& m_smtlib2Interface;

	public:
		SMTLibTranslationContext(SMTLib2Interface const& _smtlib2Interface) : m_smtlib2Interface(_smtlib2Interface) {}

		SortPointer toSort(SMTLib2Expression const& expr)
		{
			if (isAtom(expr))
			{
				auto const& name = asAtom(expr);
				if (name == "Int")
					return SortProvider::sintSort;
				if (name == "Bool")
					return SortProvider::boolSort;
				std::string quotedName = "|" + name + "|";
				auto it = ranges::find_if(m_smtlib2Interface.sortNames(), [&](auto const& entry) {
					return entry.second == name || entry.second == quotedName;
				});
				if (it != m_smtlib2Interface.sortNames().end()) {
					if (it->first->kind == Kind::Tuple) {
						auto const* tupleSort = dynamic_cast<TupleSort const*>(it->first);
						smtAssert(tupleSort);
						// TODO: This is cumbersome, we should really store shared_pointer instead of raw pointer in the sortNames
						return std::make_shared<TupleSort>(tupleSort->name, tupleSort->members, tupleSort->components);
					}
				}
			} else {
				auto const& args = asSubExpressions(expr);
				if (asAtom(args[0]) == "Array")
				{
					assert(args.size() == 3);
					auto domainSort = toSort(args[1]);
					auto codomainSort = toSort(args[2]);
					return std::make_shared<ArraySort>(std::move(domainSort), std::move(codomainSort));
				}
			}
			smtAssert(false, "Unknown sort encountered");
		}

		smtutil::Expression toSMTUtilExpression(SMTLib2Expression const& _expr)
		{
			return std::visit(GenericVisitor{
					[&](std::string const& _atom) {
						if (_atom == "true" || _atom == "false")
							return smtutil::Expression(_atom == "true");
						else if (isNumber(_atom))
							return smtutil::Expression(_atom, {}, SortProvider::sintSort);
						else
							return smtutil::Expression(_atom, {}, SortProvider::boolSort);
					},
					[&](std::vector<SMTLib2Expression> const& _subExpr) {
						SortPointer sort;
						std::vector<smtutil::Expression> arguments;
						if (isAtom(_subExpr.front()))
						{
							for (size_t i = 1; i < _subExpr.size(); i++)
								arguments.emplace_back(toSMTUtilExpression(_subExpr[i]));
							std::string const& op = asAtom(_subExpr.front());
							if (boost::starts_with(op, "struct")) {
								auto sort = toSort(_subExpr.front());
								auto sortSort = std::make_shared<SortSort>(sort);
								return Expression::tuple_constructor(Expression(sortSort), arguments);
							} else {
								std::set<std::string> boolOperators{"and", "or", "not", "=", "<", ">", "<=", ">=",
																	"=>"};
								sort = contains(boolOperators, op) ? SortProvider::boolSort : arguments.back().sort;
							}
							return smtutil::Expression(op, std::move(arguments), std::move(sort));
						} else {
							// check for const array
							if (_subExpr.size() == 2 and !isAtom(_subExpr[0]))
							{
								auto const& typeArgs = asSubExpressions(_subExpr.front());
								if (typeArgs.size() == 3 && typeArgs[0].toString() == "as" && typeArgs[1].toString() == "const")
								{
									auto arraySort = toSort(typeArgs[2]);
									auto sortSort = std::make_shared<SortSort>(arraySort);
									return smtutil::Expression::const_array(Expression(sortSort), toSMTUtilExpression(_subExpr[1]));
								}
							}

							smtAssert(false, "Unhandled case in expression conversion");
						}
					}
			}, _expr.data);
		}
	};



	class SMTLib2Parser
	{
	public:
		SMTLib2Parser(std::istream& _input):
				m_input(_input),
				m_token(static_cast<char>(m_input.get()))
		{}

		SMTLib2Expression parseExpression()
		{
			skipWhitespace();
			if (token() == '(')
			{
				advance();
				skipWhitespace();
				std::vector<SMTLib2Expression> subExpressions;
				while (token() != 0 && token() != ')')
				{
					subExpressions.emplace_back(parseExpression());
					skipWhitespace();
				}
				solAssert(token() == ')');
				// simulate whitespace because we do not want to read the next token
				// since it might block.
				m_token = ' ';
				return {std::move(subExpressions)};
			}
			else
				return {parseToken()};
		}

		bool isEOF()
		{
			skipWhitespace();
			return m_input.eof();
		}

	private:
		std::string parseToken()
		{
			std::string result;

			skipWhitespace();
			bool isPipe = token() == '|';
			if (isPipe)
				advance();
			while (token() != 0)
			{
				char c = token();
				if (isPipe && c == '|')
				{
					advance();
					break;
				}
				else if (!isPipe && (isWhiteSpace(c) || c == '(' || c == ')'))
					break;
				result.push_back(c);
				advance();
			}
			return result;
		}

		void skipWhitespace()
		{
			while (isWhiteSpace(token()))
				advance();
		}

		char token() const
		{
			return m_token;
		}

		void advance()
		{
			m_token = static_cast<char>(m_input.get());
			if (token() == ';')
				while (token() != '\n' && token() != 0)
					m_token = static_cast<char>(m_input.get());
		}

		std::istream& m_input;
		char m_token = 0;
	};

	struct LetBindings {
		using BindingRecord = std::vector<SMTLib2Expression>;
		std::unordered_map<std::string, BindingRecord> bindings;
		std::vector<std::string> varNames;
		std::vector<std::size_t> scopeBounds;

		bool has(std::string const& varName)
		{
			return bindings.find(varName) != bindings.end();
		}

		SMTLib2Expression & operator[](std::string const& varName)
		{
			auto it = bindings.find(varName);
			assert(it != bindings.end());
			assert(!it->second.empty());
			return it->second.back();
		}

		void pushScope()
		{
			scopeBounds.push_back(varNames.size());
		}

		void popScope()
		{
			assert(scopeBounds.size() > 0);
			auto bound = scopeBounds.back();
			while (varNames.size() > bound) {
				auto const& varName = varNames.back();
				auto it = bindings.find(varName);
				assert(it != bindings.end());
				auto & record = it->second;
				record.pop_back();
				if (record.empty()) {
					bindings.erase(it);
				}
				varNames.pop_back();
			}
			scopeBounds.pop_back();
		}

		void addBinding(std::string name, SMTLib2Expression expression)
		{
			auto it = bindings.find(name);
			if (it == bindings.end()) {
				bindings.insert({name, {std::move(expression)}});
			} else {
				it->second.push_back(std::move(expression));
			}
			varNames.push_back(std::move(name));
		}
	};

	void inlineLetExpressions(SMTLib2Expression& expr, LetBindings & bindings)
	{
		if (isAtom(expr))
		{
			auto const& atom = std::get<std::string>(expr.data);
			if (bindings.has(atom))
				expr = bindings[atom];
		}
		else
		{
			auto& subexprs = std::get<SMTLib2Expression::args_t>(expr.data);
			auto const& first = subexprs[0];
			if (isAtom(first) && std::get<std::string>(first.data) == "let")
			{
				assert(!isAtom(subexprs[1]));
				auto& bindingExpressions = std::get<SMTLib2Expression::args_t>(subexprs[1].data);
				// process new bindings
				std::vector<std::pair<std::string, SMTLib2Expression>> newBindings;
				for (auto& binding: bindingExpressions)
				{
					assert(!isAtom(binding));
					auto& bindingPair = std::get<SMTLib2Expression::args_t>(binding.data);
					assert(bindingPair.size() == 2);
					assert(isAtom(bindingPair.at(0)));
					inlineLetExpressions(bindingPair.at(1), bindings);
					newBindings.emplace_back(std::get<std::string>(bindingPair.at(0).data), bindingPair.at(1));
				}
				bindings.pushScope();
				for (auto&& [name, expr] : newBindings)
					bindings.addBinding(std::move(name), std::move(expr));
				newBindings.clear();

				// get new subexpression
				inlineLetExpressions(subexprs.at(2), bindings);
				// remove the new bindings
				bindings.popScope();

				// update the expression
				auto tmp = std::move(subexprs.at(2));
				expr = std::move(tmp);
				return;
			}
			// not a let expression, just process all arguments
			for (auto& subexpr: subexprs)
			{
				inlineLetExpressions(subexpr, bindings);
			}
		}
	}

	void inlineLetExpressions(SMTLib2Expression& expr)
	{
		LetBindings bindings;
		inlineLetExpressions(expr, bindings);
	}

	SMTLib2Expression const& fact(SMTLib2Expression const& _node)
	{
		if (isAtom(_node))
			return _node;
		return asSubExpressions(_node).back();
	}
}

CHCSolverInterface::CexGraph CHCSmtLib2Interface::graphFromSMTLib2Expression(SMTLib2Expression const& _proof)
{
	assert(!isAtom(_proof));
	auto const& args = asSubExpressions(_proof);
	smtAssert(args.size() == 2);
	smtAssert(isAtom(args.at(0)) && asAtom(args.at(0)) == "proof");
	auto const& proofNode = args.at(1);
	auto derivedFact = fact(proofNode);
	if (isAtom(proofNode) || !isAtom(derivedFact) || asAtom(derivedFact) != "false")
		return {};

	CHCSolverInterface::CexGraph graph;
	SMTLibTranslationContext context(*m_smtlib2);

	std::stack<SMTLib2Expression const*> proofStack;
	proofStack.push(&asSubExpressions(proofNode).at(1));

	std::map<SMTLib2Expression const*, unsigned> visitedIds;
	unsigned nextId = 0;


	auto const* root = proofStack.top();
	auto const& derivedRootFact = fact(*root);
	visitedIds.insert({root, nextId++});
	graph.nodes.emplace(visitedIds.at(root), context.toSMTUtilExpression(derivedRootFact));

	auto isHyperRes = [](SMTLib2Expression const& expr) {
		if (isAtom(expr)) return false;
		auto const& subExprs = asSubExpressions(expr);
		assert(!subExprs.empty());
		auto const& op = subExprs.at(0);
		if (isAtom(op)) return false;
		auto const& opExprs = asSubExpressions(op);
		if (opExprs.size() < 2) return false;
		auto const& ruleName = opExprs.at(1);
		return isAtom(ruleName) && asAtom(ruleName) == "hyper-res";
	};

	while (!proofStack.empty())
	{
		auto const* proofNode = proofStack.top();
		smtAssert(visitedIds.find(proofNode) != visitedIds.end(), "");
		auto id = visitedIds.at(proofNode);
		smtAssert(graph.nodes.count(id), "");
		proofStack.pop();

		if (isHyperRes(*proofNode))
		{
			auto const& args = asSubExpressions(*proofNode);
			smtAssert(args.size() > 1, "");
			// args[0] is the name of the rule
			// args[1] is the clause used
			// last argument is the derived fact
			// the arguments in the middle are the facts where we need to recurse
			for (unsigned i = 2; i < args.size() - 1; ++i)
			{
				auto const* child = &args[i];
				if (!visitedIds.count(child))
				{
					visitedIds.insert({child, nextId++});
					proofStack.push(child);
				}

				auto childId = visitedIds.at(child);
				if (!graph.nodes.count(childId))
				{
					graph.nodes.emplace(childId, context.toSMTUtilExpression(fact(*child)));
					graph.edges[childId] = {};
				}

				graph.edges[id].push_back(childId);
			}
		}
	}
	return graph;
}

CHCSolverInterface::CexGraph CHCSmtLib2Interface::graphFromZ3Proof(std::string const& _proof)
{
	std::stringstream ss(_proof);
	std::string answer;
	ss >> answer;
	solAssert(answer == "unsat");
	SMTLib2Parser parser(ss);
	solAssert(!parser.isEOF());
	// For some reason Z3 outputs everything as a single s-expression
	auto all = parser.parseExpression();
	solAssert(parser.isEOF());
	solAssert(!isAtom(all));
	auto& commands = std::get<SMTLib2Expression::args_t>(all.data);
	for (auto& command: commands) {
//		std::cout << command.toString() << '\n' << std::endl;
		if (!isAtom(command))
		{
			auto const& head = std::get<SMTLib2Expression::args_t>(command.data)[0];
			if (isAtom(head) && std::get<std::string>(head.data) == "proof")
			{
//				std::cout << "Proof expression!\n" << command.toString() << std::endl;
				inlineLetExpressions(command);
//				std::cout << "Cleaned Proof expression!\n" << command.toString() << std::endl;
				return graphFromSMTLib2Expression(command);
			}
		}
	}
	return {};
}
