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

#include <libsmtutil/Z3Interface.h>

#include <libsolutil/CommonIO.h>

using namespace std;
using namespace solidity::smtutil;

Z3Interface::Z3Interface():
	m_solver(m_context)
{
	// These need to be set globally.
	z3::set_param("rewriter.pull_cheap_ite", true);
	z3::set_param("rlimit", resourceLimit);
}

void Z3Interface::reset()
{
	m_constants.clear();
	m_functions.clear();
	m_solver.reset();
}

void Z3Interface::push()
{
	m_solver.push();
}

void Z3Interface::pop()
{
	m_solver.pop();
}

void Z3Interface::declareVariable(string const& _name, SortPointer const& _sort)
{
	smtAssert(_sort, "");
	if (_sort->kind == Kind::Function)
		declareFunction(_name, *_sort);
	else if (m_constants.count(_name))
		m_constants.at(_name) = m_context.constant(_name.c_str(), z3Sort(*_sort));
	else
		m_constants.emplace(_name, m_context.constant(_name.c_str(), z3Sort(*_sort)));
}

void Z3Interface::declareFunction(string const& _name, Sort const& _sort)
{
	smtAssert(_sort.kind == Kind::Function, "");
	FunctionSort fSort = dynamic_cast<FunctionSort const&>(_sort);
	if (m_functions.count(_name))
		m_functions.at(_name) = m_context.function(_name.c_str(), z3Sort(fSort.domain), z3Sort(*fSort.codomain));
	else
		m_functions.emplace(_name, m_context.function(_name.c_str(), z3Sort(fSort.domain), z3Sort(*fSort.codomain)));
}

void Z3Interface::addAssertion(Expression const& _expr)
{
	m_solver.add(toZ3Expr(_expr));
}

pair<CheckResult, vector<string>> Z3Interface::check(vector<Expression> const& _expressionsToEvaluate)
{
	CheckResult result;
	vector<string> values;
	try
	{
		switch (m_solver.check())
		{
		case z3::check_result::sat:
			result = CheckResult::SATISFIABLE;
			break;
		case z3::check_result::unsat:
			result = CheckResult::UNSATISFIABLE;
			break;
		case z3::check_result::unknown:
			result = CheckResult::UNKNOWN;
			break;
		}

		if (result == CheckResult::SATISFIABLE && !_expressionsToEvaluate.empty())
		{
			z3::model m = m_solver.get_model();
			for (Expression const& e: _expressionsToEvaluate)
				values.push_back(util::toString(m.eval(toZ3Expr(e))));
		}
	}
	catch (z3::exception const&)
	{
		result = CheckResult::ERROR;
		values.clear();
	}

	return make_pair(result, values);
}

z3::expr Z3Interface::toZ3Expr(Expression const& _expr)
{
	if (_expr.arguments.empty() && m_constants.count(_expr.name))
		return m_constants.at(_expr.name);
	z3::expr_vector arguments(m_context);
	for (auto const& arg: _expr.arguments)
		arguments.push_back(toZ3Expr(arg));

	try
	{
		string const& n = _expr.name;
		if (m_functions.count(n))
			return m_functions.at(n)(arguments);
		else if (m_constants.count(n))
		{
			smtAssert(arguments.empty(), "");
			return m_constants.at(n);
		}
		else if (arguments.empty())
		{
			if (n == "true")
				return m_context.bool_val(true);
			else if (n == "false")
				return m_context.bool_val(false);
			else if (_expr.sort->kind == Kind::Sort)
			{
				auto sortSort = dynamic_pointer_cast<SortSort>(_expr.sort);
				smtAssert(sortSort, "");
				return m_context.constant(n.c_str(), z3Sort(*sortSort->inner));
			}
			else
				try
				{
					return m_context.int_val(n.c_str());
				}
				catch (z3::exception const& _e)
				{
					smtAssert(false, _e.msg());
				}
		}

		smtAssert(_expr.hasCorrectArity(), "");
		if (n == "ite")
			return z3::ite(arguments[0], arguments[1], arguments[2]);
		else if (n == "not")
			return !arguments[0];
		else if (n == "and")
			return arguments[0] && arguments[1];
		else if (n == "or")
			return arguments[0] || arguments[1];
		else if (n == "implies")
			return z3::implies(arguments[0], arguments[1]);
		else if (n == "=")
			return arguments[0] == arguments[1];
		else if (n == "<")
			return arguments[0] < arguments[1];
		else if (n == "<=")
			return arguments[0] <= arguments[1];
		else if (n == ">")
			return arguments[0] > arguments[1];
		else if (n == ">=")
			return arguments[0] >= arguments[1];
		else if (n == "+")
			return arguments[0] + arguments[1];
		else if (n == "-")
			return arguments[0] - arguments[1];
		else if (n == "*")
			return arguments[0] * arguments[1];
		else if (n == "/")
			return arguments[0] / arguments[1];
		else if (n == "mod")
			return z3::mod(arguments[0], arguments[1]);
		else if (n == "bvand")
			return arguments[0] & arguments[1];
		else if (n == "int2bv")
		{
			size_t size = std::stoul(_expr.arguments[1].name);
			return z3::int2bv(size, arguments[0]);
		}
		else if (n == "bv2int")
		{
			auto intSort = dynamic_pointer_cast<IntSort>(_expr.sort);
			smtAssert(intSort, "");
			return z3::bv2int(arguments[0], intSort->isSigned);
		}
		else if (n == "select")
			return z3::select(arguments[0], arguments[1]);
		else if (n == "store")
			return z3::store(arguments[0], arguments[1], arguments[2]);
		else if (n == "const_array")
		{
			shared_ptr<SortSort> sortSort = std::dynamic_pointer_cast<SortSort>(_expr.arguments[0].sort);
			smtAssert(sortSort, "");
			auto arraySort = dynamic_pointer_cast<ArraySort>(sortSort->inner);
			smtAssert(arraySort && arraySort->domain, "");
			return z3::const_array(z3Sort(*arraySort->domain), arguments[1]);
		}
		else if (n == "tuple_get")
		{
			size_t index = stoul(_expr.arguments[1].name);
			return z3::func_decl(m_context, Z3_get_tuple_sort_field_decl(m_context, z3Sort(*_expr.arguments[0].sort), index))(arguments[0]);
		}
		else if (n == "tuple_constructor")
		{
			auto constructor = z3::func_decl(m_context, Z3_get_tuple_sort_mk_decl(m_context, z3Sort(*_expr.sort)));
			smtAssert(constructor.arity() == arguments.size(), "");
			z3::expr_vector args(m_context);
			for (auto const& arg: arguments)
				args.push_back(arg);
			return constructor(args);
		}

		smtAssert(false, "");
	}
	catch (z3::exception const& _e)
	{
		smtAssert(false, _e.msg());
	}

	smtAssert(false, "");
}

z3::sort Z3Interface::z3Sort(Sort const& _sort)
{
	switch (_sort.kind)
	{
	case Kind::Bool:
		return m_context.bool_sort();
	case Kind::Int:
		return m_context.int_sort();
	case Kind::Array:
	{
		auto const& arraySort = dynamic_cast<ArraySort const&>(_sort);
		return m_context.array_sort(z3Sort(*arraySort.domain), z3Sort(*arraySort.range));
	}
	case Kind::Tuple:
	{
		auto const& tupleSort = dynamic_cast<TupleSort const&>(_sort);
		vector<char const*> cMembers;
		for (auto const& member: tupleSort.members)
			cMembers.emplace_back(member.c_str());
		/// Using this instead of the function below because with that one
		/// we can't use `&sorts[0]` here.
		vector<z3::sort> sorts;
		for (auto const& sort: tupleSort.components)
			sorts.push_back(z3Sort(*sort));
		z3::func_decl_vector projs(m_context);
		z3::func_decl tupleConstructor = m_context.tuple_sort(
			tupleSort.name.c_str(),
			tupleSort.members.size(),
			cMembers.data(),
			sorts.data(),
			projs
		);
		return tupleConstructor.range();
	}

	default:
		break;
	}
	smtAssert(false, "");
	// Cannot be reached.
	return m_context.int_sort();
}

z3::sort_vector Z3Interface::z3Sort(vector<SortPointer> const& _sorts)
{
	z3::sort_vector z3Sorts(m_context);
	for (auto const& _sort: _sorts)
		z3Sorts.push_back(z3Sort(*_sort));
	return z3Sorts;
}
