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

#include <libsmtutil/SolverInterface.h>
#include <libsmtutil/Sorts.h>

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace solidity::smtutil
{

using SortId = uint32_t;
struct SortPairHash
{
	std::size_t operator()(std::pair<SortId, SortId> const& _pair) const;
};

struct SMTLibType {
	Kind const kind;
	SortId const id;
	SMTLibType(Kind _kind, SortId _id): kind(_kind), id(_id) {}
	virtual ~SMTLibType() = default;
};

struct SMTLibSort : public SMTLibType
{
	std::string const name;
	std::vector<SortId> const args;

	SMTLibSort(
		Kind _kind,
		std::string_view _name,
		std::vector<SortId> _args,
		SortId _id
	): SMTLibType(_kind, _id), name(_name), args(std::move(_args)) {}
};

struct TupleType : public SMTLibType
{
	std::string const name;
	std::vector<std::pair<std::string,SortId>> accessors;

	TupleType(std::string_view _name, std::vector<std::pair<std::string,SortId>> _accessors, SortId _id)
	: SMTLibType(Kind::Tuple, _id), name(_name), accessors(std::move(_accessors)) {}
};

class SMTLib2Context
{
public:
	using TupleDeclarationCallback = std::function<void(TupleSort const&)>;

	SMTLib2Context();
	void clear();

	bool isDeclared(std::string const& _name) const;
	void declare(std::string const& _name, SortPointer const& _sort);
	SortPointer getDeclaredSort(std::string const& _name) const;

	SortId resolve(SortPointer const& _sort);
	SortPointer unresolve(SortId _sortId) const;

	std::string toString(SortId _id);

	std::string toSExpr(Expression const& _expr);
	std::string toSmtLibSort(SortPointer const& _sort);

	std::optional<SortPointer> getTupleType(std::string const& _name) const;
	std::optional<std::pair<std::string, SortPointer>> getTupleAccessor(std::string const& _name) const;

	void setTupleDeclarationCallback(TupleDeclarationCallback _callback);
private:
	SortId resolveBitVectorSort(BitVectorSort const& _sort);
	SortId resolveArraySort(ArraySort const& _sort);
	SortId resolveTupleSort(TupleSort const& _sort);

	using functions_t = std::map<std::string, SortPointer>;
	functions_t m_functions; // Variables are uninterpreted constants = nullary functions

	SortId const m_boolSort{0u};
	SortId const m_intSort{1u};
	std::vector<std::unique_ptr<SMTLibType>> m_knownTypes;
	std::unordered_map<std::pair<SortId, SortId>, SortId, SortPairHash> m_arraySorts;
	std::unordered_map<std::size_t, SortId> m_bitVectorSorts;
	std::unordered_map<std::string, SortId> m_tupleSorts;

	TupleDeclarationCallback m_callback;
};

}

