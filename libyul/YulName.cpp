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

#include <libyul/YulName.h>

#include <libyul/Exceptions.h>

#include <fmt/compile.h>

namespace solidity::yul
{

YulNameRepository::YulNameRepository()
{
	auto const emptyName = defineName("");
	yulAssert(emptyName == YulNameRepository::emptyName());
}

std::string_view YulNameRepository::requiredLabelOf(YulName const& _name) const
{
	auto const label = labelOf(_name);
	yulAssert(label.has_value(), "YulName currently has no defined label in the YulNameRepository.");
	return label.value();
}

std::optional<YulNameRepository::YulName> YulNameRepository::nameOfLabel(std::string_view const _label) const
{
	auto const it = std::find(m_definedLabels.begin(), m_definedLabels.end(), _label);
	if (it != m_definedLabels.end())
	{
		YulName labelName{static_cast<YulName>(std::distance(m_definedLabels.begin(), it))};
		// mostly it'll be iota
		if (!isDerivedName(labelName) && std::get<0>(m_names[static_cast<size_t>(labelName)]) == labelName)
			return std::get<0>(m_names[static_cast<size_t>(labelName)]);
		// if not iota, we have to search
		auto itName = std::find(m_names.rbegin(), m_names.rend(), std::make_tuple(labelName, YulNameState::DEFINED));
		if (itName != m_names.rend())
			return std::get<0>(*itName);
	}
	return std::nullopt;
}

void YulNameRepository::generateLabels(std::set<YulName> const& _usedNames, std::set<std::string> const& _illegal)
{
	std::set<std::string> used;
	std::set<YulName> toDerive;
	for (auto const name: _usedNames)
	{
		if (!isDerivedName(name))
		{
			auto const label = labelOf(name);
			yulAssert(label.has_value());
			auto const [it, emplaced] = used.emplace(*label);
			if (!emplaced || _illegal.count(*it) > 0)
				// there's been a clash ,e.g., by calling generate labels twice;
				// let's remove this name and derive it instead
				toDerive.insert(name);
		}
		else
			yulAssert(isDerivedName(name) || _illegal.count(std::string(*labelOf(name))) == 0);
	}

	std::vector<std::tuple<std::string, YulName>> generated;
	auto namesIt = _usedNames.begin();
	for (size_t nameValue = 1; nameValue < m_names.size(); ++nameValue)
	{
		auto name = static_cast<YulName>(nameValue);
		if (namesIt != _usedNames.end() && name == *namesIt)
		{
			if (isDerivedName(name) || toDerive.find(name) != toDerive.end())
			{
				std::string const baseLabel(baseLabelOf(name));
				std::string label (baseLabel);
				size_t bump = 1;
				while (used.count(label) > 0 || _illegal.count(label) > 0)
					label = fmt::format(FMT_COMPILE("{}_{}"), baseLabel, bump++);
				if (auto const existingDefinedName = nameOfLabel(label); existingDefinedName.has_value())
				{
					std::get<0>(m_names[static_cast<size_t>(name)]) = *existingDefinedName;
					std::get<1>(m_names[static_cast<size_t>(name)]) = YulNameState::DEFINED;
				}
				else
					generated.emplace_back(label, name);
				used.insert(label);
			}
			++namesIt;
		}
	}

	for (auto const& [label, name] : generated)
	{
		yulAssert(_illegal.count(label) == 0);
		m_definedLabels.emplace_back(label);
		std::get<0>(m_names[static_cast<size_t>(name)]) = static_cast<YulName>(m_definedLabels.size() - 1);
		std::get<1>(m_names[static_cast<size_t>(name)]) = YulNameState::DEFINED;
	}
}

}
