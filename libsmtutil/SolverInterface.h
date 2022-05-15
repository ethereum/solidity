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

#pragma once

#include <libsmtutil/Exceptions.h>
#include <libsmtutil/Sorts.h>

#include <libsolutil/Common.h>
#include <libsolutil/Numeric.h>
#include <libsolutil/CommonData.h>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/view.hpp>

#include <cstdio>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace solidity::smtutil
{

struct SMTSolverChoice
{
	bool cvc4 = false;
	bool eld = false;
	bool smtlib2 = false;
	bool z3 = false;

	static constexpr SMTSolverChoice All() noexcept { return {true, true, true, true}; }
	static constexpr SMTSolverChoice CVC4() noexcept { return {true, false, false, false}; }
	static constexpr SMTSolverChoice ELD() noexcept { return {false, true, false, false}; }
	static constexpr SMTSolverChoice SMTLIB2() noexcept { return {false, false, true, false}; }
	static constexpr SMTSolverChoice Z3() noexcept { return {false, false, false, true}; }
	static constexpr SMTSolverChoice None() noexcept { return {false, false, false, false}; }

	static std::optional<SMTSolverChoice> fromString(std::string const& _solvers)
	{
		SMTSolverChoice solvers;
		if (_solvers == "all")
		{
			smtAssert(solvers.setSolver("cvc4"), "");
			smtAssert(solvers.setSolver("eld"), "");
			smtAssert(solvers.setSolver("smtlib2"), "");
			smtAssert(solvers.setSolver("z3"), "");
		}
		else
			for (auto&& s: _solvers | ranges::views::split(',') | ranges::to<std::vector<std::string>>())
				if (!solvers.setSolver(s))
					return {};

		return solvers;
	}

	SMTSolverChoice& operator&=(SMTSolverChoice const& _other)
	{
		cvc4 &= _other.cvc4;
		eld &= _other.eld;
		smtlib2 &= _other.smtlib2;
		z3 &= _other.z3;
		return *this;
	}

	SMTSolverChoice operator&(SMTSolverChoice _other) const noexcept
	{
		_other &= *this;
		return _other;
	}

	bool operator!=(SMTSolverChoice const& _other) const noexcept { return !(*this == _other); }

	bool operator==(SMTSolverChoice const& _other) const noexcept
	{
		return cvc4 == _other.cvc4 &&
			eld == _other.eld &&
			smtlib2 == _other.smtlib2 &&
			z3 == _other.z3;
	}

	bool setSolver(std::string const& _solver)
	{
		static std::set<std::string> const solvers{"cvc4", "eld", "smtlib2", "z3"};
		if (!solvers.count(_solver))
			return false;
		if (_solver == "cvc4")
			cvc4 = true;
		if (_solver == "eld")
			eld = true;
		else if (_solver == "smtlib2")
			smtlib2 = true;
		else if (_solver == "z3")
			z3 = true;
		return true;
	}

	bool none() const noexcept { return !some(); }
	bool some() const noexcept { return cvc4 || eld || smtlib2 || z3; }
	bool all() const noexcept { return cvc4 && eld && smtlib2 && z3; }
};

enum class CheckResult
{
	SATISFIABLE, UNSATISFIABLE, UNKNOWN, CONFLICTING, ERROR
};

/// C++ representation of an SMTLIB2 expression.
class Expression
{
	friend class SolverInterface;
public:
	explicit Expression(bool _v): Expression(_v ? "true" : "false", Kind::Bool) {}
	explicit Expression(std::shared_ptr<SortSort> _sort, std::string _name = ""): Expression(std::move(_name), {}, _sort) {}
	explicit Expression(std::string _name, std::vector<Expression> _arguments, SortPointer _sort):
		name(std::move(_name)), arguments(std::move(_arguments)), sort(std::move(_sort)) {}
	Expression(size_t _number): Expression(std::to_string(_number), {}, SortProvider::sintSort) {}
	Expression(u256 const& _number): Expression(_number.str(), {}, SortProvider::sintSort) {}
	Expression(s256 const& _number): Expression(
		_number >= 0 ? _number.str() : "-",
		_number >= 0 ?
			std::vector<Expression>{} :
			std::vector<Expression>{Expression(size_t(0)), bigint(-_number)},
		SortProvider::sintSort
	) {}
	Expression(bigint const& _number): Expression(
		_number >= 0 ? _number.str() : "-",
		_number >= 0 ?
			std::vector<Expression>{} :
			std::vector<Expression>{Expression(size_t(0)), bigint(-_number)},
		SortProvider::sintSort
	) {}

	Expression(Expression const&) = default;
	Expression(Expression&&) = default;
	Expression& operator=(Expression const&) = default;
	Expression& operator=(Expression&&) = default;

	bool hasCorrectArity() const
	{
		if (name == "tuple_constructor")
		{
			auto tupleSort = std::dynamic_pointer_cast<TupleSort>(sort);
			smtAssert(tupleSort, "");
			return arguments.size() == tupleSort->components.size();
		}

		static std::map<std::string, unsigned> const operatorsArity{
			{"ite", 3},
			{"not", 1},
			{"and", 2},
			{"or", 2},
			{"=>", 2},
			{"=", 2},
			{"<", 2},
			{"<=", 2},
			{">", 2},
			{">=", 2},
			{"+", 2},
			{"-", 2},
			{"*", 2},
			{"div", 2},
			{"mod", 2},
			{"bvnot", 1},
			{"bvand", 2},
			{"bvor", 2},
			{"bvxor", 2},
			{"bvshl", 2},
			{"bvlshr", 2},
			{"bvashr", 2},
			{"int2bv", 2},
			{"bv2int", 1},
			{"select", 2},
			{"store", 3},
			{"const_array", 2},
			{"tuple_get", 2}
		};
		return operatorsArity.count(name) && operatorsArity.at(name) == arguments.size();
	}

	static Expression ite(Expression _condition, Expression _trueValue, Expression _falseValue)
	{
		smtAssert(*_trueValue.sort == *_falseValue.sort, "");
		SortPointer sort = _trueValue.sort;
		return Expression("ite", std::vector<Expression>{
			std::move(_condition), std::move(_trueValue), std::move(_falseValue)
		}, std::move(sort));
	}

	static Expression implies(Expression _a, Expression _b)
	{
		return Expression(
			"=>",
			std::move(_a),
			std::move(_b),
			Kind::Bool
		);
	}

	/// select is the SMT representation of an array index access.
	static Expression select(Expression _array, Expression _index)
	{
		smtAssert(_array.sort->kind == Kind::Array, "");
		std::shared_ptr<ArraySort> arraySort = std::dynamic_pointer_cast<ArraySort>(_array.sort);
		smtAssert(arraySort, "");
		smtAssert(_index.sort, "");
		smtAssert(*arraySort->domain == *_index.sort, "");
		return Expression(
			"select",
			std::vector<Expression>{std::move(_array), std::move(_index)},
			arraySort->range
		);
	}

	/// store is the SMT representation of an assignment to array index.
	/// The function is pure and returns the modified array.
	static Expression store(Expression _array, Expression _index, Expression _element)
	{
		auto arraySort = std::dynamic_pointer_cast<ArraySort>(_array.sort);
		smtAssert(arraySort, "");
		smtAssert(_index.sort, "");
		smtAssert(_element.sort, "");
		smtAssert(*arraySort->domain == *_index.sort, "");
		smtAssert(*arraySort->range == *_element.sort, "");
		return Expression(
			"store",
			std::vector<Expression>{std::move(_array), std::move(_index), std::move(_element)},
			arraySort
		);
	}

	static Expression const_array(Expression _sort, Expression _value)
	{
		smtAssert(_sort.sort->kind == Kind::Sort, "");
		auto sortSort = std::dynamic_pointer_cast<SortSort>(_sort.sort);
		auto arraySort = std::dynamic_pointer_cast<ArraySort>(sortSort->inner);
		smtAssert(sortSort && arraySort, "");
		smtAssert(_value.sort, "");
		smtAssert(*arraySort->range == *_value.sort, "");
		return Expression(
			"const_array",
			std::vector<Expression>{std::move(_sort), std::move(_value)},
			arraySort
		);
	}

	static Expression tuple_get(Expression _tuple, size_t _index)
	{
		smtAssert(_tuple.sort->kind == Kind::Tuple, "");
		std::shared_ptr<TupleSort> tupleSort = std::dynamic_pointer_cast<TupleSort>(_tuple.sort);
		smtAssert(tupleSort, "");
		smtAssert(_index < tupleSort->components.size(), "");
		return Expression(
			"tuple_get",
			std::vector<Expression>{std::move(_tuple), Expression(_index)},
			tupleSort->components.at(_index)
		);
	}

	static Expression tuple_constructor(Expression _tuple, std::vector<Expression> _arguments)
	{
		smtAssert(_tuple.sort->kind == Kind::Sort, "");
		auto sortSort = std::dynamic_pointer_cast<SortSort>(_tuple.sort);
		auto tupleSort = std::dynamic_pointer_cast<TupleSort>(sortSort->inner);
		smtAssert(tupleSort, "");
		smtAssert(_arguments.size() == tupleSort->components.size(), "");
		return Expression(
			"tuple_constructor",
			std::move(_arguments),
			tupleSort
		);
	}

	static Expression int2bv(Expression _n, size_t _size)
	{
		smtAssert(_n.sort->kind == Kind::Int, "");
		std::shared_ptr<IntSort> intSort = std::dynamic_pointer_cast<IntSort>(_n.sort);
		smtAssert(intSort, "");
		smtAssert(_size <= 256, "");
		return Expression(
			"int2bv",
			std::vector<Expression>{std::move(_n), Expression(_size)},
			std::make_shared<BitVectorSort>(_size)
		);
	}

	static Expression bv2int(Expression _bv, bool _signed = false)
	{
		smtAssert(_bv.sort->kind == Kind::BitVector, "");
		std::shared_ptr<BitVectorSort> bvSort = std::dynamic_pointer_cast<BitVectorSort>(_bv.sort);
		smtAssert(bvSort, "");
		smtAssert(bvSort->size <= 256, "");
		return Expression(
			"bv2int",
			std::vector<Expression>{std::move(_bv)},
			SortProvider::intSort(_signed)
		);
	}

	static bool sameSort(std::vector<Expression> const& _args)
	{
		if (_args.empty())
			return true;

		auto sort = _args.front().sort;
		return ranges::all_of(
			_args,
			[&](auto const& _expr){ return _expr.sort->kind == sort->kind; }
		);
	}

	static Expression mkAnd(std::vector<Expression> _args)
	{
		smtAssert(!_args.empty(), "");
		smtAssert(sameSort(_args), "");

		auto sort = _args.front().sort;
		if (sort->kind == Kind::BitVector)
			return Expression("bvand", std::move(_args), sort);

		smtAssert(sort->kind == Kind::Bool, "");
		return Expression("and", std::move(_args), Kind::Bool);
	}

	static Expression mkOr(std::vector<Expression> _args)
	{
		smtAssert(!_args.empty(), "");
		smtAssert(sameSort(_args), "");

		auto sort = _args.front().sort;
		if (sort->kind == Kind::BitVector)
			return Expression("bvor", std::move(_args), sort);

		smtAssert(sort->kind == Kind::Bool, "");
		return Expression("or", std::move(_args), Kind::Bool);
	}

	static Expression mkPlus(std::vector<Expression> _args)
	{
		smtAssert(!_args.empty(), "");
		smtAssert(sameSort(_args), "");

		auto sort = _args.front().sort;
		smtAssert(sort->kind == Kind::BitVector || sort->kind == Kind::Int, "");
		return Expression("+", std::move(_args), sort);
	}

	static Expression mkMul(std::vector<Expression> _args)
	{
		smtAssert(!_args.empty(), "");
		smtAssert(sameSort(_args), "");

		auto sort = _args.front().sort;
		smtAssert(sort->kind == Kind::BitVector || sort->kind == Kind::Int, "");
		return Expression("*", std::move(_args), sort);
	}

	friend Expression operator!(Expression _a)
	{
		if (_a.sort->kind == Kind::BitVector)
			return ~_a;
		return Expression("not", std::move(_a), Kind::Bool);
	}
	friend Expression operator&&(Expression _a, Expression _b)
	{
		if (_a.sort->kind == Kind::BitVector)
		{
			smtAssert(_b.sort->kind == Kind::BitVector, "");
			return _a & _b;
		}
		return Expression("and", std::move(_a), std::move(_b), Kind::Bool);
	}
	friend Expression operator||(Expression _a, Expression _b)
	{
		if (_a.sort->kind == Kind::BitVector)
		{
			smtAssert(_b.sort->kind == Kind::BitVector, "");
			return _a | _b;
		}
		return Expression("or", std::move(_a), std::move(_b), Kind::Bool);
	}
	friend Expression operator==(Expression _a, Expression _b)
	{
		smtAssert(_a.sort->kind == _b.sort->kind, "Trying to create an 'equal' expression with different sorts");
		return Expression("=", std::move(_a), std::move(_b), Kind::Bool);
	}
	friend Expression operator!=(Expression _a, Expression _b)
	{
		return !(std::move(_a) == std::move(_b));
	}
	friend Expression operator<(Expression _a, Expression _b)
	{
		return Expression("<", std::move(_a), std::move(_b), Kind::Bool);
	}
	friend Expression operator<=(Expression _a, Expression _b)
	{
		return Expression("<=", std::move(_a), std::move(_b), Kind::Bool);
	}
	friend Expression operator>(Expression _a, Expression _b)
	{
		return Expression(">", std::move(_a), std::move(_b), Kind::Bool);
	}
	friend Expression operator>=(Expression _a, Expression _b)
	{
		return Expression(">=", std::move(_a), std::move(_b), Kind::Bool);
	}
	friend Expression operator+(Expression _a, Expression _b)
	{
		auto intSort = _a.sort;
		return Expression("+", {std::move(_a), std::move(_b)}, intSort);
	}
	friend Expression operator-(Expression _a, Expression _b)
	{
		auto intSort = _a.sort;
		return Expression("-", {std::move(_a), std::move(_b)}, intSort);
	}
	friend Expression operator*(Expression _a, Expression _b)
	{
		auto intSort = _a.sort;
		return Expression("*", {std::move(_a), std::move(_b)}, intSort);
	}
	friend Expression operator/(Expression _a, Expression _b)
	{
		auto intSort = _a.sort;
		return Expression("div", {std::move(_a), std::move(_b)}, intSort);
	}
	friend Expression operator%(Expression _a, Expression _b)
	{
		auto intSort = _a.sort;
		return Expression("mod", {std::move(_a), std::move(_b)}, intSort);
	}
	friend Expression operator~(Expression _a)
	{
		auto bvSort = _a.sort;
		return Expression("bvnot", {std::move(_a)}, bvSort);
	}
	friend Expression operator&(Expression _a, Expression _b)
	{
		auto bvSort = _a.sort;
		return Expression("bvand", {std::move(_a), std::move(_b)}, bvSort);
	}
	friend Expression operator|(Expression _a, Expression _b)
	{
		auto bvSort = _a.sort;
		return Expression("bvor", {std::move(_a), std::move(_b)}, bvSort);
	}
	friend Expression operator^(Expression _a, Expression _b)
	{
		auto bvSort = _a.sort;
		return Expression("bvxor", {std::move(_a), std::move(_b)}, bvSort);
	}
	friend Expression operator<<(Expression _a, Expression _b)
	{
		auto bvSort = _a.sort;
		return Expression("bvshl", {std::move(_a), std::move(_b)}, bvSort);
	}
	friend Expression operator>>(Expression _a, Expression _b)
	{
		auto bvSort = _a.sort;
		return Expression("bvlshr", {std::move(_a), std::move(_b)}, bvSort);
	}
	static Expression ashr(Expression _a, Expression _b)
	{
		auto bvSort = _a.sort;
		return Expression("bvashr", {std::move(_a), std::move(_b)}, bvSort);
	}
	Expression operator()(std::vector<Expression> _arguments) const
	{
		smtAssert(
			sort->kind == Kind::Function,
			"Attempted function application to non-function."
		);
		auto fSort = dynamic_cast<FunctionSort const*>(sort.get());
		smtAssert(fSort, "");
		return Expression(name, std::move(_arguments), fSort->codomain);
	}

	std::string name;
	std::vector<Expression> arguments;
	SortPointer sort;

private:
	/// Manual constructors, should only be used by SolverInterface and this class itself.
	Expression(std::string _name, std::vector<Expression> _arguments, Kind _kind):
		Expression(std::move(_name), std::move(_arguments), std::make_shared<Sort>(_kind)) {}

	explicit Expression(std::string _name, Kind _kind):
		Expression(std::move(_name), std::vector<Expression>{}, _kind) {}
	Expression(std::string _name, Expression _arg, Kind _kind):
		Expression(std::move(_name), std::vector<Expression>{std::move(_arg)}, _kind) {}
	Expression(std::string _name, Expression _arg1, Expression _arg2, Kind _kind):
		Expression(std::move(_name), std::vector<Expression>{std::move(_arg1), std::move(_arg2)}, _kind) {}
};

DEV_SIMPLE_EXCEPTION(SolverError);

class SolverInterface
{
public:
	SolverInterface(std::optional<unsigned> _queryTimeout = {}): m_queryTimeout(_queryTimeout) {}

	virtual ~SolverInterface() = default;
	virtual void reset() = 0;

	virtual void push() = 0;
	virtual void pop() = 0;

	virtual void declareVariable(std::string const& _name, SortPointer const& _sort) = 0;
	Expression newVariable(std::string _name, SortPointer const& _sort)
	{
		// Subclasses should do something here
		smtAssert(_sort, "");
		declareVariable(_name, _sort);
		return Expression(std::move(_name), {}, _sort);
	}

	virtual void addAssertion(Expression const& _expr) = 0;

	/// Checks for satisfiability, evaluates the expressions if a model
	/// is available. Throws SMTSolverError on error.
	virtual std::pair<CheckResult, std::vector<std::string>>
	check(std::vector<Expression> const& _expressionsToEvaluate) = 0;

	/// @returns a list of queries that the system was not able to respond to.
	virtual std::vector<std::string> unhandledQueries() { return {}; }

	/// @returns how many SMT solvers this interface has.
	virtual size_t solvers() { return 1; }

protected:
	std::optional<unsigned> m_queryTimeout;
};

}
