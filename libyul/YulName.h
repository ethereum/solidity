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

#include <libyul/ControlFlowSideEffects.h>
#include <libyul/SideEffects.h>
#include <libyul/YulString.h>

#include <fmt/format.h>

#include <unordered_map>
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <functional>
#include <numeric>
#include <map>

namespace solidity::yul
{

enum class LiteralKind;
struct Dialect;
struct EVMDialect;
struct Block;
struct BuiltinFunction;

class YulNameRepository
{
public:
	using YulName = size_t;

	/// Decorating a yul dialect builtin function with `YulName`s.
	struct BuiltinFunction
	{
		YulName name;
		std::vector<YulName> parameters;
		std::vector<YulName> returns;

		yul::BuiltinFunction const* data;
	};

	struct PredefinedHandles
	{
		YulName empty {};
		YulName verbatim {};
		YulName boolType {};
		YulName hashFunction {};
		YulName datasize {};
		YulName dataoffset {};
		YulName selfdestruct {};
		YulName memoryguard {};
		YulName eq {};
		YulName add {};
		YulName sub {};
		YulName tstore {};
		YulName defaultType {};
		YulName placeholder_zero {};
		YulName placeholder_one {};
		YulName placeholder_thirtytwo {};
	};

	/// Construct via dialect. It is important that the dialect instance lives at least as long as the name repository
	/// instance.
	explicit YulNameRepository(Dialect const& _dialect);
	~YulNameRepository() = default;
	YulNameRepository(YulNameRepository&&) = default;
	YulNameRepository& operator=(YulNameRepository&&) = default;
	YulNameRepository(YulNameRepository const&) = default;
	YulNameRepository& operator=(YulNameRepository const&) = default;

	/// Defines a new name based on a label. If the label already was defined, it returns the corresponding YulName
	/// instead of a new one.
	YulName defineName(std::string_view _label);

	/// Defines a new name based on a parent name. When generating labels, the generated label will be based on the
	/// parent's.
	YulName deriveName(YulName _id);

	/// Adds a ghost name.
	YulName addGhost();

	/// The empty name.
	static constexpr YulName emptyName() { return 0; }

	/// Yields the label of a yul name. The name must have been added via ``defineName``, a label must have been
	/// generated with ``generateLabels``, or it is a builtin.
	std::optional<std::string_view> labelOf(YulName _name) const;

	/// Yields the name that the provided name was based on - or the name itself, if the name was directly "defined".
	YulName baseNameOf(YulName _name) const;

	/// Yields the label of the base name of the provided name. Opposed to ``labelOf``, this must always exist.
	std::string_view baseLabelOf(YulName _name) const;

	/// Whether a name is considered derived, i.e., has no label but a parent name.
	bool isDerivedName(YulName const _name) const { return std::get<1>(m_names[_name]) == YulNameState::DERIVED; }

	/// Whether a name corresponds to a verbatim builtin function.
	bool isVerbatimFunction(YulName _name) const;

	/// A couple of predefined names.
	PredefinedHandles const& predefined() const { return m_predefined; }

	/// Functionality that decorates a yul dialect based on YulNames (ids).
	[[nodiscard]] BuiltinFunction const* builtin(YulName _name) const;
	bool isBuiltinName(YulName _name) const;

	BuiltinFunction const* discardFunction(YulName _type) const;
	BuiltinFunction const* equalityFunction(YulName _type) const;
	BuiltinFunction const* booleanNegationFunction() const;

	BuiltinFunction const* memoryStoreFunction(YulName _type) const;
	BuiltinFunction const* memoryLoadFunction(YulName _type) const;
	BuiltinFunction const* storageStoreFunction(YulName _type) const;
	BuiltinFunction const* storageLoadFunction(YulName _type) const;
	YulName hashFunction(YulName _type) const;

	/// Tries to find the label in the defined names and returns the corresponding name. If not found, ``emptyName``.
	YulName nameOfLabel(std::string_view label) const;

	/// Same as nameOfLabel, but restricted to builtins and therefore computationally more efficient.
	YulName nameOfBuiltin(std::string_view builtin) const;

	/// Same as nameOfLabel, but restricted to types and therefore computationally more efficient.
	YulName nameOfType(std::string_view type) const;

	/// Whether the name is corresponding to a type.
	bool isType(YulName _name) const;

	/// Number of types. If the dialect is untyped, there is still one type (the "empty type" type)
	size_t nTypes() const;

	Dialect const& dialect() const;

	bool isEvmDialect() const;

	EVMDialect const* evmDialect() const;

	/// Generates labels for derived names over the set of _usedNames, respecting a set of _illegal labels.
	/// This will change the state of all derived names in _usedNames to "not derived" with a label associated to them.
	void generateLabels(std::set<YulName> const& _usedNames, std::set<std::string> const& _illegal = {});

	// commented out for the time being, as it requires AST refactoring to be YulName-based
	// void generateLabels(Block const& _ast, std::set<std::string> const& _illegal = {});

private:
	struct PredefinedBuiltinFunctions
	{
		std::vector<std::optional<YulName>> discardFunctions;
		std::vector<std::optional<YulName>> equalityFunctions;
		std::optional<YulName> booleanNegationFunction;
		std::vector<std::optional<YulName>> memoryStoreFunctions;
		std::vector<std::optional<YulName>> memoryLoadFunctions;
		std::vector<std::optional<YulName>> storageStoreFunctions;
		std::vector<std::optional<YulName>> storageLoadFunctions;
		std::vector<YulName> hashFunctions;
	};
	struct IndexBoundaries
	{
		size_t beginTypes {};
		size_t endTypes {};
		size_t beginBuiltins {};
		size_t endBuiltins {};
	};
	enum class YulNameState	{ DERIVED, DEFINED };
	bool nameWithinBounds(YulName const _name) const { return _name < m_index; }

	size_t indexOfType(YulName _type) const;
	BuiltinFunction convertBuiltinFunction(YulName _name, yul::BuiltinFunction const& _builtin) const;
	BuiltinFunction const* fetchTypedPredefinedFunction(YulName _type, std::vector<std::optional<YulName>> const& _functions) const;

	std::reference_wrapper<Dialect const> m_dialect;
	EVMDialect const* m_evmDialect;
	std::vector<std::tuple<YulName, std::string>> m_dialectTypes;
	std::map<YulName, BuiltinFunction> m_builtinFunctions;

	PredefinedBuiltinFunctions m_predefinedBuiltinFunctions;

	size_t m_index {0};
	size_t m_nGhosts {0};
	std::vector<std::string> m_definedLabels {};
	std::vector<std::tuple<YulName, YulNameState>> m_names {};
	std::map<std::tuple<size_t, size_t>, YulName> m_verbatimNames {};
	PredefinedHandles m_predefined{};
	IndexBoundaries m_indexBoundaries;
};
using YulName = YulString;

}
