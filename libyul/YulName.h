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

#include <libyul/SideEffects.h>
#include <libyul/YulString.h>

#include <fmt/format.h>

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <functional>

namespace solidity::yul
{

enum class LiteralKind;
struct Block;

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
 */
class YulNameRepository
{
public:
	using YulName = std::uint64_t;
	YulNameRepository();
	/// Defines a new name based on a label. If the label already was defined, it returns the corresponding YulName
	/// instead of a new one (`defineName("xyz") == defineName("xyz")`).
	YulName defineName(std::string_view _label)
	{
		if (auto const name = nameOfLabel(_label); name.has_value())
			return *name;

		m_definedLabels.emplace_back(_label);
		m_names.emplace_back(m_definedLabels.size() - 1, YulNameState::DEFINED);
		return static_cast<YulName>(m_names.size() - 1);
	}

	/// Defines a new name based on a parent name. When generating labels, the generated label will be based on the
	/// parent's (`deriveName(id) != deriveName(id)`).
	YulName deriveName(YulName const& _baseName)
	{
		m_names.emplace_back(baseNameOf(_baseName), YulNameState::DERIVED);
		return static_cast<YulName>(m_names.size() - 1);
	}

	/// The empty name.
	static constexpr YulName emptyName() { return 0; }

	/// Yields the label of a yul name. The name must have been added via `defineName`, a label must have been
	/// generated with `generateLabels`, or it is a builtin.
	std::optional<std::string_view> labelOf(YulName _name) const
	{
		if (isDerivedName(_name))
			return std::nullopt;

		auto const labelIndex = std::get<0>(m_names.at(static_cast<size_t>(_name)));
		return m_definedLabels.at(static_cast<size_t>(labelIndex));
	}

	/// If it can be assumed that the label was already generated, this function will yield it (or fail with an
	/// assertion error).
	std::string_view requiredLabelOf(YulName const& _name) const;

	/// Yields the label of the base name of the provided name. Opposed to `labelOf`, this must always exist.
	std::string_view baseLabelOf(YulName const& _name) const
	{
		auto const labelIndex = std::get<0>(m_names.at(static_cast<size_t>(baseNameOf(_name))));
		return m_definedLabels[static_cast<size_t>(labelIndex)];
	}

	/// Whether a name is considered derived, i.e., has no label but a parent name.
	bool isDerivedName(YulName const& _name) const
	{
		return std::get<1>(m_names.at(static_cast<size_t>(_name))) == YulNameState::DERIVED;
	}

	/// Tries to find the label in the defined names and returns the corresponding name.
	std::optional<YulName> nameOfLabel(std::string_view label) const;

	/// Generates labels for derived names over the set of _usedNames, respecting a set of _illegal labels.
	/// This will change the state of all derived names in _usedNames to "not derived" with a label associated to them.
	void generateLabels(std::set<YulName> const& _usedNames, std::set<std::string> const& _illegal = {});

private:
	enum class YulNameState	{ DERIVED, DEFINED };

	/// Yields the name that the provided name was based on - or the name itself, if the name was directly "defined".
	YulName baseNameOf(YulName _name) const
	{
		while (isDerivedName(_name))
			_name = std::get<0>(m_names[static_cast<size_t>(_name)]);
		return _name;
	}

	std::vector<std::string> m_definedLabels{};
	std::vector<std::tuple<YulName, YulNameState>> m_names{};
};
using YulName = YulString;

}
