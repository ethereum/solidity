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

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/AST.h>
#include <libyul/Dialect.h>
#include <libyul/Exceptions.h>

#include <fmt/compile.h>

namespace solidity::yul
{

YulNameRepository::YulNameRepository(solidity::yul::Dialect const& _dialect):
	m_dialect(_dialect),
	m_evmDialect(dynamic_cast<EVMDialect const*>(&_dialect))
{
	m_predefined.empty = defineName("");
	yulAssert(m_predefined.empty == emptyName());

	for (auto const& type: _dialect.types)
		if (type.empty())
		{
			m_indexBoundaries.beginTypes = 0;
			m_dialectTypes.emplace_back(emptyName(), type);
		}
		else
		{
			m_indexBoundaries.beginTypes = 1;
			m_dialectTypes.emplace_back(defineName(type), type);
		}
	m_indexBoundaries.endTypes = m_names.size();
	m_indexBoundaries.beginBuiltins = m_names.size();

	auto const& builtinNames = _dialect.builtinNames();
	m_predefined.verbatim = defineName("@ verbatim");
	for (auto const& label: builtinNames)
		if (!label.empty())
		{
			auto const name = defineName(label);
			if (auto const* function = m_dialect.get().builtin(label))
				m_builtinFunctions[name] = convertBuiltinFunction(name, *function);
		}
	m_indexBoundaries.endBuiltins = m_names.size();

	m_predefined.boolType = nameOfType(_dialect.boolType);
	m_predefined.defaultType = nameOfType(_dialect.defaultType);

	auto const predefinedName = [&](std::string const& label)
	{
		if (builtinNames.count(label) > 0)
			return nameOfBuiltin(label);
		else
			return defineName(label);
	};
	m_predefined.dataoffset = predefinedName("dataoffset");
	m_predefined.datasize = predefinedName("datasize");
	m_predefined.selfdestruct = predefinedName("selfdestruct");
	m_predefined.tstore = predefinedName("tstore");
	m_predefined.memoryguard = predefinedName("memoryguard");
	m_predefined.eq = predefinedName("eq");
	m_predefined.add = predefinedName("add");
	m_predefined.sub = predefinedName("sub");

	{
		auto types = m_dialectTypes;
		if (types.empty())
			types.emplace_back(YulName{0, m_instanceCounter.value}, "");

		auto nameOfBuiltinIfAvailable = [this](yul::BuiltinFunction const* _builtin) -> std::optional<YulName>
		{
			if (_builtin)
				return nameOfBuiltin(_builtin->name);
			return std::nullopt;
		};
		m_predefinedBuiltinFunctions.booleanNegationFunction = nameOfBuiltinIfAvailable(m_dialect.get().booleanNegationFunction());
		for (auto const& [typeName, typeLabel]: types)
		{
			m_predefinedBuiltinFunctions.discardFunctions.emplace_back(nameOfBuiltinIfAvailable(m_dialect.get().discardFunction(typeLabel)));
			m_predefinedBuiltinFunctions.equalityFunctions.emplace_back(nameOfBuiltinIfAvailable(m_dialect.get().equalityFunction(typeLabel)));
			m_predefinedBuiltinFunctions.memoryStoreFunctions.emplace_back(nameOfBuiltinIfAvailable(m_dialect.get().memoryStoreFunction(typeLabel)));
			m_predefinedBuiltinFunctions.memoryLoadFunctions.emplace_back(nameOfBuiltinIfAvailable(m_dialect.get().memoryLoadFunction(typeLabel)));
			m_predefinedBuiltinFunctions.storageStoreFunctions.emplace_back(nameOfBuiltinIfAvailable(m_dialect.get().storageStoreFunction(typeLabel)));
			m_predefinedBuiltinFunctions.storageLoadFunctions.emplace_back(nameOfBuiltinIfAvailable(m_dialect.get().storageLoadFunction(typeLabel)));
			m_predefinedBuiltinFunctions.hashFunctions.emplace_back(nameOfBuiltin(m_dialect.get().hashFunction(typeLabel)));
		}
	}

	m_predefined.placeholderZero = defineName("@ 0");
	m_predefined.placeholderOne = defineName("@ 1");
	m_predefined.placeholderThirtyTwo = defineName("@ 32");
}

YulNameRepository::BuiltinFunctionWrapper const* YulNameRepository::builtin(YulName const& _name) const
{
	assertCompatibility(_name);
	auto const baseName = baseNameOf(_name);
	if (isBuiltinName(baseName))
	{
		auto const it = m_builtinFunctions.find(_name);
		if (it != m_builtinFunctions.end())
			return &it->second;
	}
	return nullptr;
}

std::optional<std::string_view> YulNameRepository::labelOf(YulName const& _name) const
{
	assertCompatibility(_name);
	if (!isDerivedName(_name))
	{
		// if the parent is directly a defined label, we take that one
		yulAssert(_name.value <= std::numeric_limits<size_t>::max());
		YulName::ValueType const labelIndex = std::get<0>(m_names[static_cast<size_t>(_name.value)]).value;
		yulAssert(labelIndex <= std::numeric_limits<size_t>::max());
		yulAssert(static_cast<size_t>(labelIndex) < m_definedLabels.size());
		return m_definedLabels[static_cast<size_t>(labelIndex)];
	}
	if (isVerbatimFunction(_name))
	{
		auto const* builtinFun = builtin(_name);
		yulAssert(builtinFun);
		return builtinFun->definition->name;
	}
	return std::nullopt;
}

std::string_view YulNameRepository::requiredLabelOf(YulName const& _name) const
{
	auto const label = labelOf(_name);
	yulAssert(label.has_value(), "YulName currently has no defined label in the YulNameRepository.");
	return label.value();
}

YulNameRepository::YulName const& YulNameRepository::baseNameOf(YulName const& _name) const
{
	assertCompatibility(_name);
	YulName const* result = &_name;
	yulAssert(result->value <= std::numeric_limits<size_t>::max());
	while (isDerivedName(*result))
		result = &std::get<0>(m_names[static_cast<size_t>(result->value)]);
	return *result;
}

std::string_view YulNameRepository::baseLabelOf(YulName const& _name) const
{
	assertCompatibility(_name);
	YulName::ValueType const labelIndex = std::get<0>(m_names[static_cast<size_t>(baseNameOf(_name).value)]).value;
	yulAssert(labelIndex <= std::numeric_limits<size_t>::max());
	return m_definedLabels[static_cast<size_t>(labelIndex)];
}

YulNameRepository::BuiltinFunctionWrapper const* YulNameRepository::fetchTypedPredefinedFunction(YulName const& _type, std::vector<std::optional<YulName>> const& _functions) const
{
	assertCompatibility(_type);
	auto const typeIndex = indexOfType(_type);
	yulAssert(typeIndex < _functions.size());
	auto const& functionName = _functions[typeIndex];
	if (!functionName)
		return nullptr;
	return builtin(*functionName);
}

YulNameRepository::BuiltinFunctionWrapper const* YulNameRepository::discardFunction(YulName const& _type) const
{
	return fetchTypedPredefinedFunction(_type, m_predefinedBuiltinFunctions.discardFunctions);
}

YulNameRepository::BuiltinFunctionWrapper const* YulNameRepository::equalityFunction(YulName const& _type) const
{
	return fetchTypedPredefinedFunction(_type, m_predefinedBuiltinFunctions.equalityFunctions);
}

YulNameRepository::BuiltinFunctionWrapper const* YulNameRepository::booleanNegationFunction() const
{
	if (!m_predefinedBuiltinFunctions.booleanNegationFunction)
		return nullptr;
	return builtin(*m_predefinedBuiltinFunctions.booleanNegationFunction);
}

YulNameRepository::BuiltinFunctionWrapper const* YulNameRepository::memoryLoadFunction(YulName const& _type) const
{
	return fetchTypedPredefinedFunction(_type, m_predefinedBuiltinFunctions.memoryLoadFunctions);
}

YulNameRepository::BuiltinFunctionWrapper const* YulNameRepository::memoryStoreFunction(YulName const& _type) const
{
	return fetchTypedPredefinedFunction(_type, m_predefinedBuiltinFunctions.memoryStoreFunctions);
}

YulNameRepository::BuiltinFunctionWrapper const* YulNameRepository::storageLoadFunction(YulName const& _type) const
{
	return fetchTypedPredefinedFunction(_type, m_predefinedBuiltinFunctions.storageLoadFunctions);
}

YulNameRepository::BuiltinFunctionWrapper const* YulNameRepository::storageStoreFunction(YulName const& _type) const
{
	return fetchTypedPredefinedFunction(_type, m_predefinedBuiltinFunctions.storageStoreFunctions);
}

YulNameRepository::YulName const& YulNameRepository::hashFunction(YulName const& _type) const
{
	assertCompatibility(_type);
	auto const typeIndex = indexOfType(_type);
	return m_predefinedBuiltinFunctions.hashFunctions[typeIndex];
}

bool YulNameRepository::isBuiltinName(YulName const& _name) const
{
	assertCompatibility(_name);
	auto const baseName = baseNameOf(_name);
	return baseName.value >= m_indexBoundaries.beginBuiltins && baseName.value < m_indexBoundaries.endBuiltins;
}

YulNameRepository::BuiltinFunctionWrapper
YulNameRepository::convertBuiltinFunction(YulName const& _name, yul::BuiltinFunction const& _builtin) const
{
	assertCompatibility(_name);
	BuiltinFunctionWrapper result;
	result.name = _name;
	for (auto const& type: _builtin.parameters)
		result.parameters.push_back(nameOfType(type));
	for (auto const& type: _builtin.returns)
		result.returns.push_back(nameOfType(type));
	result.definition = &_builtin;
	return result;
}

YulNameRepository::YulName const& YulNameRepository::nameOfLabel(std::string_view const _label) const
{
	yulAssert(!_label.empty());
	auto const it = std::find(m_definedLabels.begin(), m_definedLabels.end(), _label);
	if (it != m_definedLabels.end())
	{
		YulName labelName{static_cast<YulName::ValueType>(std::distance(m_definedLabels.begin(), it)), m_instanceCounter.value};
		yulAssert(labelName.value <= std::numeric_limits<size_t>::max());
		// mostly it'll be iota
		if (!isDerivedName(labelName) && std::get<0>(m_names[static_cast<size_t>(labelName.value)]) == labelName)
			return std::get<0>(m_names[static_cast<size_t>(labelName.value)]);
		// if not iota, we have to search
		auto itName = std::find(m_names.rbegin(), m_names.rend(), std::make_tuple(labelName, YulNameState::DEFINED));
		if (itName != m_names.rend())
			return std::get<0>(*itName);
	}
	return emptyName();
}

YulNameRepository::YulName const& YulNameRepository::nameOfBuiltin(std::string_view const _builtin) const
{
	for (size_t i = m_indexBoundaries.beginBuiltins; i < m_indexBoundaries.endBuiltins; ++i)
		if (baseLabelOf(std::get<0>(m_names[i])) == _builtin)
			return std::get<0>(m_names[i]);
	return emptyName();
}

YulNameRepository::YulName const& YulNameRepository::nameOfType(std::string_view const _type) const
{
	if (!m_dialectTypes.empty())
	{
		for (auto const& m_dialectType: m_dialectTypes)
			if (std::get<1>(m_dialectType) == _type)
				return std::get<0>(m_dialectType);
		yulAssert(false, "only defined for (some) dialect types");
	}
	else
		return emptyName();
}

size_t YulNameRepository::indexOfType(YulName const& _type) const
{
	if (m_dialectTypes.empty())
		return 0;
	auto const it = std::find_if(m_dialectTypes.begin(), m_dialectTypes.end(), [&](auto const& element) { return std::get<0>(element) == _type; });
	yulAssert(it != m_dialectTypes.end(), "tried to get index of unknown type");
	return static_cast<size_t>(std::distance(m_dialectTypes.begin(), it));
}

Dialect const& YulNameRepository::dialect() const
{
	return m_dialect;
}

bool YulNameRepository::isVerbatimFunction(YulName const& _name) const
{
	assertCompatibility(_name);
	return baseNameOf(_name) == predefined().verbatim;
}

YulNameRepository::YulName YulNameRepository::defineName(std::string_view const _label)
{
	if (auto const* builtin = m_dialect.get().builtin(_label))
	{
		if (builtin->name.substr(0, std::string_view("verbatim").size()) == "verbatim")
		{
			auto const key = std::make_tuple(builtin->parameters.size(), builtin->returns.size());
			auto [it, emplaced] = m_verbatimNames.try_emplace(key);
			if (emplaced)
			{
				it->second = deriveName(predefined().verbatim);
				m_builtinFunctions[it->second] = convertBuiltinFunction(it->second, *builtin);
			}
			return it->second;
		}
		else
		{
			auto const builtinName = nameOfBuiltin(_label);
			if (builtinName == emptyName())
			{
				m_definedLabels.emplace_back(_label);
				m_names.emplace_back(YulName{m_definedLabels.size() - 1, m_instanceCounter.value}, YulNameState::DEFINED);
				return YulName{m_names.size() - 1, m_instanceCounter.value};
			}
			else
				return builtinName;
		}
	}
	else
	{
		if (!m_names.empty())
			if (auto const name = nameOfLabel(_label); name != emptyName())
				return name;

		m_definedLabels.emplace_back(_label);
		m_names.emplace_back(YulName{m_definedLabels.size() - 1, m_instanceCounter.value}, YulNameState::DEFINED);
		return YulName{m_names.size() - 1, m_instanceCounter.value};
	}
}

YulNameRepository::YulName YulNameRepository::deriveName(YulName const& _name)
{
	m_names.emplace_back(_name, YulNameState::DERIVED);
	return YulName{m_names.size() - 1, m_instanceCounter.value};
}

bool YulNameRepository::isType(YulName const& _name) const {
	return _name.value >= m_indexBoundaries.beginTypes && _name.value < m_indexBoundaries.endTypes;
}

bool YulNameRepository::isDerivedName(YulName const& _name) const
{
	assertCompatibility(_name);
	yulAssert(_name.value <= std::numeric_limits<size_t>::max());
	return std::get<1>(m_names[static_cast<size_t>(_name.value)]) == YulNameState::DERIVED;
}

size_t YulNameRepository::typeCount() const
{
	return m_indexBoundaries.endTypes - m_indexBoundaries.beginTypes;
}

bool YulNameRepository::isEvmDialect() const { return dynamic_cast<EVMDialect const*>(&m_dialect.get()) != nullptr; }

EVMDialect const* YulNameRepository::evmDialect() const { return m_evmDialect; }

void YulNameRepository::generateLabels(std::set<YulName> const& _usedNames, std::set<std::string> const& _illegal)
{
	std::set<std::string> used (m_definedLabels.begin(), m_definedLabels.begin() + static_cast<std::ptrdiff_t>(m_indexBoundaries.endBuiltins));
	std::set<YulName> toDerive;
	for (auto const name: _usedNames)
	{
		if (!isDerivedName(name) || isVerbatimFunction(name))
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
	for (size_t nameValue = m_indexBoundaries.endBuiltins; nameValue < m_names.size(); ++nameValue)
	{
		YulName name{nameValue, m_instanceCounter.value};
		if (namesIt != _usedNames.end() && name == *namesIt)
		{
			if ((isDerivedName(name) && !isVerbatimFunction(name)) || toDerive.find(name) != toDerive.end())
			{
				std::string const baseLabel(baseLabelOf(name));
				std::string label (baseLabel);
				size_t bump = 1;
				while (used.count(label) > 0 || _illegal.count(label) > 0)
					label = fmt::format(FMT_COMPILE("{}_{}"), baseLabel, bump++);
				if (auto const& existingDefinedName = nameOfLabel(label); existingDefinedName != emptyName() || name == emptyName())
				{
					std::get<0>(m_names[static_cast<size_t>(name.value)]).value = existingDefinedName.value;
					std::get<1>(m_names[static_cast<size_t>(name.value)]) = YulNameState::DEFINED;
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
		std::get<0>(m_names[static_cast<size_t>(name.value)]).value = m_definedLabels.size() - 1;
		std::get<1>(m_names[static_cast<size_t>(name.value)]) = YulNameState::DEFINED;
	}
}

void YulNameRepository::assertCompatibility(YulName const& _name) const
{
	bool const compatible = _name.value < m_names.size();
	yulAssert(compatible, "YulName incompatible with repository (value too large or different origin).");
}

void YulNameRepository::generateLabels(Block const& _ast, std::set<std::string> const& _illegal)
{
	generateLabels(NameCollector(_ast).names(), _illegal);
}

std::uint32_t YulNameRepository::InstanceCounter::count = 0;

}
