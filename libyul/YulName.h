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

/**
 * Yul Name repository.
 *
 * A database of numerical identifiers for Yul nodes in an AST (`YulName`s). Identifiers can be
 *
 *  - 'defined', i.e., they are equipped with a string label, which can be retrieved by a call to `labelOf(yulName)`;
 *  - 'derived', i.e., they don't possess a string label but have a parent yul name which is either also derived or
 *    defined. All dependency chains of derived labels terminate in a defined label.
 *
 * Such derived identifiers can be introduced, e.g., during certain optimization steps. If the AST (or segments thereof)
 * should be printed and/or the string label of identifiers should be retrieved which are still in derived state,
 * a call to `generateLabels(identifiers)` changes the status of all derived labels to defined and generates
 * unique labels for all identifiers provided to the method. The generated labels are based on their parents.
 *
 * The second purpose of the YulNameRepository is acting as a proxy for a `yul::Dialect`. Builtins as well
 * as types are mapped to entries in the repository and can be queried using corresponding methods.
 *
 * There is the special case of verbatim functions.
 * verbatim functions are builtin functions which are already variable, therefore they are derived from a common
 * ancestor `PredefinedHandles.verbatim` but don't need a label generated for them, as the label is already
 * defined by their number of inputs and outputs
 */
class YulNameRepository
{
public:
	struct YulName
	{
		using ValueType = std::uint64_t;
		ValueType value{};
		std::uint32_t repositoryInstanceId{};

		bool operator==(YulName const& other) const
		{
			if (other.empty() && empty())
				return true;
			return repositoryInstanceId == other.repositoryInstanceId && value == other.value;
		}
		bool operator!=(YulName const& other) const
		{
			return !(*this == other);
		}
		bool operator<(YulName const& _other) const
		{
			if (_other.empty() && empty())
				return false;
			if (repositoryInstanceId < _other.repositoryInstanceId)
				return true;
			return value < _other.value;
		}

		[[nodiscard]] bool empty() const { return value == 0; }
	};

	/// Wrapping a yul dialect builtin function with `YulName`s.
	struct BuiltinFunctionWrapper
	{
		YulName name;
		std::vector<YulName> parameters;
		std::vector<YulName> returns;

		yul::BuiltinFunction const* definition;
	};

	/// A couple of predefined yul names, most of which are dialect builtins.
	struct PredefinedHandles
	{
		YulName empty{}; ///< Special empty name (corresponding to the empty label).
		YulName verbatim{}; ///< Verbatim builtin parent name. All verbatims are derived from this.
		YulName boolType{}; ///< Bool type. Depending on dialect, this may be zero/empty.
		YulName datasize{}; ///< Datasize builtin.
		YulName dataoffset{}; ///< Dataoffset builtin.
		YulName selfdestruct{}; ///< Selfdestruct builtin.
		YulName memoryguard{}; ///< Memoryguard builtin.
		YulName eq{}; ///< Equality function.
		YulName add{}; ///< Addition function.
		YulName sub{}; ///< Subtraction function.
		YulName tstore{}; ///< Tstore builtin.
		YulName defaultType{}; ///< Dialect's default type. May be zero/empty.
		YulName placeholderZero{}; ///< Special name `@ 0` used in the `UnusedStoreEliminator`.
		YulName placeholderOne{}; ///< Special name `@ 1` used in the `UnusedStoreEliminator`.
		YulName placeholderThirtyTwo{}; ///< Special name `@ 32` used in the `UnusedStoreEliminator`.
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
	/// instead of a new one (`defineName("xyz") == defineName("xyz")`.
	YulName defineName(std::string_view _label);

	/// Defines a new name based on a parent name. When generating labels, the generated label will be based on the
	/// parent's (`deriveName(id) != deriveName(id)`).
	YulName deriveName(YulName const& _baseName);

	/// The empty name.
	static constexpr YulName const& emptyName()	{ return m_emptyName; }

	/// Yields the label of a yul name. The name must have been added via `defineName`, a label must have been
	/// generated with `generateLabels`, or it is a builtin.
	std::optional<std::string_view> labelOf(YulName const& _name) const;

	/// If it can be assumed that the label was already generated, this function will yield it (or fail with an
	/// assertion error).
	std::string_view requiredLabelOf(YulName const& _name) const;

	/// Yields the name that the provided name was based on - or the name itself, if the name was directly "defined".
	YulName const& baseNameOf(YulName const& _name) const;

	/// Yields the label of the base name of the provided name. Opposed to `labelOf`, this must always exist.
	std::string_view baseLabelOf(YulName const& _name) const;

	/// Whether a name is considered derived, i.e., has no label but a parent name.
	[[nodiscard]] bool isDerivedName(YulName const& _name) const;

	/// Whether a name corresponds to a verbatim builtin function.
	[[nodiscard]] bool isVerbatimFunction(YulName const& _name) const;

	/// A couple of predefined names.
	PredefinedHandles const& predefined() const { return m_predefined; }

	/// Functionality that decorates a yul dialect based on YulNames (ids).
	[[nodiscard]] BuiltinFunctionWrapper const* builtin(YulName const& _name) const;
	bool isBuiltinName(YulName const& _name) const;

	BuiltinFunctionWrapper const* discardFunction(YulName const& _type) const;
	BuiltinFunctionWrapper const* equalityFunction(YulName const& _type) const;
	BuiltinFunctionWrapper const* booleanNegationFunction() const;

	BuiltinFunctionWrapper const* memoryStoreFunction(YulName const& _type) const;
	BuiltinFunctionWrapper const* memoryLoadFunction(YulName const& _type) const;
	BuiltinFunctionWrapper const* storageStoreFunction(YulName const& _type) const;
	BuiltinFunctionWrapper const* storageLoadFunction(YulName const& _type) const;
	YulName const& hashFunction(YulName const& _name) const;

	/// Tries to find the label in the defined names and returns the corresponding name. If not found, `emptyName`.
	YulName const& nameOfLabel(std::string_view label) const;

	/// Same as nameOfLabel, but restricted to builtins and therefore computationally more efficient.
	YulName const& nameOfBuiltin(std::string_view builtin) const;

	/// Same as nameOfLabel, but restricted to types and therefore computationally more efficient.
	YulName const& nameOfType(std::string_view type) const;

	/// Whether the name is corresponding to a type.
	[[nodiscard]] bool isType(YulName const& _name) const;

	/// Number of types. If the dialect is untyped, there is still one type (the "empty type" type)
	size_t typeCount() const;

	/// The contained dialect.
	Dialect const& dialect() const;

	/// Whether the contained dialect is an EVM dialect (see `EVMDialect`).
	[[nodiscard]] bool isEvmDialect() const;

	EVMDialect const* evmDialect() const;

	std::uint32_t instanceId() const { return m_instanceCounter.value; }

	/// Generates labels for derived names over the set of _usedNames, respecting a set of _illegal labels.
	/// This will change the state of all derived names in _usedNames to "not derived" with a label associated to them.
	void generateLabels(std::set<YulName> const& _usedNames, std::set<std::string> const& _illegal = {});
	void generateLabels(Block const& _ast, std::set<std::string> const& _illegal = {});

private:
	struct InstanceCounter
	{
		InstanceCounter(): value(++count) {}
		~InstanceCounter() = default;
		InstanceCounter(InstanceCounter const&) { value = ++count; }
		InstanceCounter& operator=(InstanceCounter const&)
		{
			value = ++count;
			return *this;
		}
		InstanceCounter(InstanceCounter&& _other) = default;
		InstanceCounter& operator=(InstanceCounter&&) = default;

		static std::uint32_t count;
		std::uint32_t value;
	};
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
		size_t beginTypes{};
		size_t endTypes{};
		size_t beginBuiltins{};
		size_t endBuiltins{};
	};
	enum class YulNameState	{ DERIVED, DEFINED };
	void assertCompatibility(YulName const& _name) const;

	size_t indexOfType(YulName const& _type) const;
	BuiltinFunctionWrapper convertBuiltinFunction(YulName const& _name, yul::BuiltinFunction const& _builtin) const;
	BuiltinFunctionWrapper const* fetchTypedPredefinedFunction(YulName const& _type, std::vector<std::optional<YulName>> const& _functions) const;

	static constexpr auto m_emptyName = YulName{0, 0};
	InstanceCounter m_instanceCounter;
	std::reference_wrapper<Dialect const> m_dialect;
	EVMDialect const* m_evmDialect;
	std::vector<std::tuple<YulName, std::string>> m_dialectTypes;
	std::map<YulName, BuiltinFunctionWrapper> m_builtinFunctions;

	PredefinedBuiltinFunctions m_predefinedBuiltinFunctions;

	std::vector<std::string> m_definedLabels{};
	std::vector<std::tuple<YulName, YulNameState>> m_names{};
	std::map<std::tuple<size_t, size_t>, YulName> m_verbatimNames{};
	PredefinedHandles m_predefined{};
	IndexBoundaries m_indexBoundaries;
};
using YulName = YulNameRepository::YulName;
using Type = YulName;

}

template <>
struct std::hash<solidity::yul::YulName>
{
	std::size_t operator()(solidity::yul::YulName const& key) const
	{
		return std::hash<solidity::yul::YulName::ValueType>{}(key.value);
	}
};
