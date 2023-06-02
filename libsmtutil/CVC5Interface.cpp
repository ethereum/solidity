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

#include <libsmtutil/CVC5Interface.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/CommonIO.h>
#include <libsolutil/Exceptions.h>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::smtutil;

CVC5Interface::CVC5Interface(optional<unsigned> _queryTimeout):
	SolverInterface(_queryTimeout)
{
	reset();
}

void CVC5Interface::reset()
{
	m_variables.clear();
	m_solver.resetAssertions();
	m_solver.setOption("produce-models", "true");
	if (m_queryTimeout)
		m_solver.setOption("tlimit-per", std::to_string(*m_queryTimeout));
	else
		m_solver.setOption("rlimit", std::to_string(resourceLimit));
}

void CVC5Interface::push()
{
	m_solver.push();
}

void CVC5Interface::pop()
{
	m_solver.pop();
}

void CVC5Interface::declareVariable(string const& _name, SortPointer const& _sort)
{
	smtAssert(_sort, "");
	m_variables[_name] = m_solver.mkConst(CVC5Sort(*_sort), _name);
}

void CVC5Interface::addAssertion(Expression const& _expr)
{
	try
	{
		m_solver.assertFormula(toCVC5Expr(_expr));
	}
	catch (cvc5::CVC5ApiException const& _e)
	{
		smtAssert(false, _e.what());
	}
}

pair<CheckResult, vector<string>> CVC5Interface::check(vector<Expression> const& _expressionsToEvaluate)
{
	CheckResult result;
	vector<string> values;
	try
	{
		cvc5::Result res = m_solver.checkSat();
		if (res.isSat())
			result = CheckResult::SATISFIABLE;
		else if (res.isUnsat())
			result = CheckResult::UNSATISFIABLE;
		else if (res.isUnknown())
			result = CheckResult::UNKNOWN;
		else
			smtAssert(false, "");
		if (result == CheckResult::SATISFIABLE && !_expressionsToEvaluate.empty())
		{
			for (Expression const& e: _expressionsToEvaluate)
				values.push_back(toString(m_solver.getValue(toCVC5Expr(e))));
		}
	}
	catch (cvc5::CVC5ApiException const&)
	{
		result = CheckResult::ERROR;
		values.clear();
	}

	return make_pair(result, values);
}

cvc5::Term CVC5Interface::toCVC5Expr(Expression const& _expr)
{
	// Variable
	if (_expr.arguments.empty() && m_variables.count(_expr.name))
		return m_variables.at(_expr.name);

	vector<cvc5::Term> arguments;
	for (auto const& arg: _expr.arguments)
		arguments.push_back(toCVC5Expr(arg));

	try
	{
		string const& n = _expr.name;
		// Function application
		if (!arguments.empty() && m_variables.count(_expr.name))
			return m_solver.mkTerm(cvc5::Kind::APPLY_UF, vector<cvc5::Term>{m_variables.at(n)} + arguments);
		// Literal
		else if (arguments.empty())
		{
			if (n == "true")
				return m_solver.mkTrue();
			else if (n == "false")
				return m_solver.mkFalse();
			else if (auto sortSort = dynamic_pointer_cast<SortSort>(_expr.sort))
				return m_solver.mkVar(CVC5Sort(*sortSort->inner), n);
			else
				try
				{
					return m_solver.mkInteger(n);
				}
				catch (cvc5::CVC5ApiRecoverableException const& _e)
				{
					smtAssert(false, _e.what());
				}
				catch (cvc5::CVC5ApiException const& _e)
				{
					smtAssert(false, _e.what());
				}
		}

		smtAssert(_expr.hasCorrectArity(), "");
		if (n == "ite")
			return arguments[0].iteTerm(arguments[1], arguments[2]);
		else if (n == "not")
			return arguments[0].notTerm();
		else if (n == "and")
			return arguments[0].andTerm(arguments[1]);
		else if (n == "or")
			return arguments[0].orTerm(arguments[1]);
		else if (n == "=>")
			return m_solver.mkTerm(cvc5::Kind::IMPLIES, {arguments[0], arguments[1]});
		else if (n == "=")
			return m_solver.mkTerm(cvc5::Kind::EQUAL, {arguments[0], arguments[1]});
		else if (n == "<")
			return m_solver.mkTerm(cvc5::Kind::LT, {arguments[0], arguments[1]});
		else if (n == "<=")
			return m_solver.mkTerm(cvc5::Kind::LEQ, {arguments[0], arguments[1]});
		else if (n == ">")
			return m_solver.mkTerm(cvc5::Kind::GT, {arguments[0], arguments[1]});
		else if (n == ">=")
			return m_solver.mkTerm(cvc5::Kind::GEQ, {arguments[0], arguments[1]});
		else if (n == "+")
			return m_solver.mkTerm(cvc5::Kind::ADD, {arguments[0], arguments[1]});
		else if (n == "-")
			return m_solver.mkTerm(cvc5::Kind::SUB, {arguments[0], arguments[1]});
		else if (n == "*")
			return m_solver.mkTerm(cvc5::Kind::MULT, {arguments[0], arguments[1]});
		else if (n == "div")
			return m_solver.mkTerm(cvc5::Kind::INTS_DIVISION, {arguments[0], arguments[1]});
		else if (n == "mod")
			return m_solver.mkTerm(cvc5::Kind::INTS_MODULUS, {arguments[0], arguments[1]});
		else if (n == "bvnot")
			return m_solver.mkTerm(cvc5::Kind::BITVECTOR_NOT, {arguments[0]});
		else if (n == "bvand")
			return m_solver.mkTerm(cvc5::Kind::BITVECTOR_AND, {arguments[0], arguments[1]});
		else if (n == "bvor")
			return m_solver.mkTerm(cvc5::Kind::BITVECTOR_OR, {arguments[0], arguments[1]});
		else if (n == "bvxor")
			return m_solver.mkTerm(cvc5::Kind::BITVECTOR_XOR, {arguments[0], arguments[1]});
		else if (n == "bvshl")
			return m_solver.mkTerm(cvc5::Kind::BITVECTOR_SHL, {arguments[0], arguments[1]});
		else if (n == "bvlshr")
			return m_solver.mkTerm(cvc5::Kind::BITVECTOR_LSHR, {arguments[0], arguments[1]});
		else if (n == "bvashr")
			return m_solver.mkTerm(cvc5::Kind::BITVECTOR_ASHR, {arguments[0], arguments[1]});
		else if (n == "int2bv")
		{
			size_t size = std::stoul(_expr.arguments[1].name);
			auto i2bvOp = m_solver.mkOp(cvc5::Kind::INT_TO_BITVECTOR, {static_cast<unsigned>(size)});
			// CVC5 treats all BVs as unsigned, so we need to manually apply 2's complement if needed.
			return m_solver.mkTerm(
				cvc5::Kind::ITE,
				{
					m_solver.mkTerm(cvc5::Kind::GEQ, {arguments[0], m_solver.mkInteger(0)}),
					m_solver.mkTerm(i2bvOp, {arguments[0]}),
					m_solver.mkTerm(
						cvc5::Kind::BITVECTOR_NEG,
						{m_solver.mkTerm(i2bvOp, {m_solver.mkTerm(cvc5::Kind::NEG, {arguments[0]})})}
					)
				}
			);
		}
		else if (n == "bv2int")
		{
			auto intSort = dynamic_pointer_cast<IntSort>(_expr.sort);
			smtAssert(intSort, "");
			auto nat = m_solver.mkTerm(cvc5::Kind::BITVECTOR_TO_NAT, {arguments[0]});
			if (!intSort->isSigned)
				return nat;

			auto type = arguments[0].getSort();
			smtAssert(type.isBitVector(), "");
			auto size = type.getBitVectorSize();
			// CVC5 treats all BVs as unsigned, so we need to manually apply 2's complement if needed.
			auto extractOp = m_solver.mkOp(cvc5::Kind::BITVECTOR_EXTRACT, {size - 1, size - 1});
			return m_solver.mkTerm(
				cvc5::Kind::ITE,
				{
					m_solver.mkTerm(
						cvc5::Kind::EQUAL,
						{m_solver.mkTerm(extractOp, {arguments[0]}), m_solver.mkBitVector(1, uint64_t{0})}
					),
					nat,
					m_solver.mkTerm(
						cvc5::Kind::NEG,
						{m_solver.mkTerm(cvc5::Kind::BITVECTOR_TO_NAT, {m_solver.mkTerm(cvc5::Kind::BITVECTOR_NEG, {arguments[0]})})}
					)
				}
			);
		}
		else if (n == "select")
			return m_solver.mkTerm(cvc5::Kind::SELECT, {arguments[0], arguments[1]});
		else if (n == "store")
			return m_solver.mkTerm(cvc5::Kind::STORE, {arguments[0], arguments[1], arguments[2]});
		else if (n == "const_array")
		{
			shared_ptr<SortSort> sortSort = std::dynamic_pointer_cast<SortSort>(_expr.arguments[0].sort);
			smtAssert(sortSort, "");
			return m_solver.mkConstArray(CVC5Sort(*sortSort->inner), arguments[1]);
		}
		else if (n == "tuple_get")
		{
			shared_ptr<TupleSort> tupleSort = std::dynamic_pointer_cast<TupleSort>(_expr.arguments[0].sort);
			smtAssert(tupleSort, "");
			auto tt = m_solver.mkTupleSort(CVC5Sort(tupleSort->components));
			cvc5::Datatype const& dt = tt.getDatatype();
			size_t index = std::stoul(_expr.arguments[1].name);
			cvc5::DatatypeSelector s = dt[0][index];
			return m_solver.mkTerm(cvc5::Kind::APPLY_SELECTOR, {s.getTerm(), arguments[0]});
		}
		else if (n == "tuple_constructor")
		{
			shared_ptr<TupleSort> tupleSort = std::dynamic_pointer_cast<TupleSort>(_expr.sort);
			smtAssert(tupleSort, "");
			auto tt = m_solver.mkTupleSort(CVC5Sort(tupleSort->components));
			cvc5::Datatype const& dt = tt.getDatatype();
			cvc5::DatatypeConstructor c = dt[0];
			return m_solver.mkTerm(cvc5::Kind::APPLY_CONSTRUCTOR, vector<cvc5::Term>{c.getTerm()} + arguments);
		}

		smtAssert(false);
	}
	catch (cvc5::CVC5ApiException const& _e)
	{
		smtAssert(false, _e.what());
	}

	smtAssert(false);

	// FIXME: Workaround for spurious GCC 12.1 warning (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105794)
	util::unreachable();
}

cvc5::Sort CVC5Interface::CVC5Sort(Sort const& _sort)
{
	switch (_sort.kind)
	{
	case Kind::Bool:
		return m_solver.getBooleanSort();
	case Kind::Int:
		return m_solver.getIntegerSort();
	case Kind::BitVector:
		return m_solver.mkBitVectorSort(dynamic_cast<BitVectorSort const&>(_sort).size);
	case Kind::Function:
	{
		FunctionSort const& fSort = dynamic_cast<FunctionSort const&>(_sort);
		if (fSort.domain.empty()) // function sort in cvc5 must have nonempty domain
			return CVC5Sort(*fSort.codomain);
		return m_solver.mkFunctionSort(CVC5Sort(fSort.domain), CVC5Sort(*fSort.codomain));
	}
	case Kind::Array:
	{
		auto const& arraySort = dynamic_cast<ArraySort const&>(_sort);
		return m_solver.mkArraySort(CVC5Sort(*arraySort.domain), CVC5Sort(*arraySort.range));
	}
	case Kind::Tuple:
	{
		auto const& tupleSort = dynamic_cast<TupleSort const&>(_sort);
		return m_solver.mkTupleSort(CVC5Sort(tupleSort.components));
	}
	default:
		break;
	}
	smtAssert(false, "");
	// Cannot be reached.
	return m_solver.getIntegerSort();
}

vector<cvc5::Sort> CVC5Interface::CVC5Sort(vector<SortPointer> const& _sorts)
{
	vector<cvc5::Sort> CVC5Sorts;
	for (auto const& _sort: _sorts)
		CVC5Sorts.push_back(CVC5Sort(*_sort));
	return CVC5Sorts;
}
