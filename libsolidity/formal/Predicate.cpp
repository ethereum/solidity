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

#include <libsolidity/formal/Predicate.h>

#include <libsolidity/ast/AST.h>

#include <boost/algorithm/string/join.hpp>
#include <utility>

using namespace std;
using namespace solidity;
using namespace solidity::smtutil;
using namespace solidity::frontend;
using namespace solidity::frontend::smt;

map<string, Predicate> Predicate::m_predicates;

Predicate const* Predicate::create(
	SortPointer _sort,
	string _name,
	EncodingContext& _context,
	ASTNode const* _node
)
{
	smt::SymbolicFunctionVariable predicate{_sort, move(_name), _context};
	string functorName = predicate.currentName();
	solAssert(!m_predicates.count(functorName), "");
	return &m_predicates.emplace(
		std::piecewise_construct,
		std::forward_as_tuple(functorName),
		std::forward_as_tuple(move(predicate), _node)
	).first->second;
}

Predicate::Predicate(
	smt::SymbolicFunctionVariable&& _predicate,
	ASTNode const* _node
):
	m_predicate(move(_predicate)),
	m_node(_node)
{
}

Predicate const* Predicate::predicate(string const& _name)
{
	return &m_predicates.at(_name);
}

void Predicate::reset()
{
	m_predicates.clear();
}

smtutil::Expression Predicate::operator()(vector<smtutil::Expression> const& _args) const
{
	return m_predicate(_args);
}

smtutil::Expression Predicate::functor() const
{
	return m_predicate.currentFunctionValue();
}

smtutil::Expression Predicate::functor(unsigned _idx) const
{
	return m_predicate.functionValueAtIndex(_idx);
}

void Predicate::newFunctor()
{
	m_predicate.increaseIndex();
}

ASTNode const* Predicate::programNode() const {
	return m_node;
}
