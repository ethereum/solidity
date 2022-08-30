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

#include <libsolidity/formal/ArraySlicePredicate.h>

#include <liblangutil/Exceptions.h>

using namespace std;
using namespace solidity;
using namespace solidity::smtutil;
using namespace solidity::frontend;
using namespace solidity::frontend::smt;

map<string, ArraySlicePredicate::SliceData> ArraySlicePredicate::m_slicePredicates;

pair<bool, ArraySlicePredicate::SliceData const&> ArraySlicePredicate::create(SortPointer _sort, EncodingContext& _context)
{
	solAssert(_sort->kind == Kind::Tuple, "");
	auto tupleSort = dynamic_pointer_cast<TupleSort>(_sort);
	solAssert(tupleSort, "");

	auto tupleName = tupleSort->name;
	if (m_slicePredicates.count(tupleName))
		return {true, m_slicePredicates.at(tupleName)};

	auto sort = tupleSort->components.at(0);
	solAssert(sort->kind == Kind::Array, "");

	smt::SymbolicArrayVariable aVar{sort, "a_" + tupleName, _context };
	smt::SymbolicArrayVariable bVar{sort, "b_" + tupleName, _context};
	smt::SymbolicIntVariable startVar{TypeProvider::uint256(), TypeProvider::uint256(), "start_" + tupleName, _context};
	smt::SymbolicIntVariable endVar{TypeProvider::uint256(), TypeProvider::uint256(), "end_" + tupleName, _context };
	smt::SymbolicIntVariable iVar{TypeProvider::uint256(), TypeProvider::uint256(), "i_" + tupleName, _context};

	vector<SortPointer> domain{sort, sort, startVar.sort(), endVar.sort()};
	auto sliceSort = make_shared<FunctionSort>(domain, SortProvider::boolSort);
	Predicate const& slice = *Predicate::create(sliceSort, "array_slice_" + tupleName, PredicateType::Custom, _context);

	domain.emplace_back(iVar.sort());
	auto predSort = make_shared<FunctionSort>(domain, SortProvider::boolSort);
	Predicate const& header = *Predicate::create(predSort, "array_slice_header_" + tupleName, PredicateType::Custom, _context);
	Predicate const& loop = *Predicate::create(predSort, "array_slice_loop_" + tupleName, PredicateType::Custom, _context);

	auto a = aVar.elements();
	auto b = bVar.elements();
	auto start = startVar.currentValue();
	auto end = endVar.currentValue();
	auto i = iVar.currentValue();

	auto rule1 = smtutil::Expression::implies(
		end > start,
		header({a, b, start, end, 0})
	);

	auto rule2 = smtutil::Expression::implies(
		header({a, b, start, end, i}) && i >= (end - start),
		slice({a, b, start, end})
	);

	auto rule3 = smtutil::Expression::implies(
		header({a, b, start, end, i}) && i >= 0 && i < (end - start),
		loop({a, b, start, end, i})
	);

	auto b_i = smtutil::Expression::select(b, i);
	auto a_start_i = smtutil::Expression::select(a, start + i);
	auto rule4 = smtutil::Expression::implies(
		loop({a, b, start, end, i}) && b_i == a_start_i,
		header({a, b, start, end, i + 1})
	);

	return {false, m_slicePredicates[tupleName] = {
		{&slice, &header, &loop},
		{std::move(rule1), std::move(rule2), std::move(rule3), std::move(rule4)}
	}};
}
