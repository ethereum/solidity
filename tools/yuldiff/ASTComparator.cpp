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

#include <tools/yuldiff/ASTComparator.h>

#include <libyul/AST.h>

#include <fmt/ranges.h>

using namespace solidity::tools::cmpast;

ASTComparator::ASTComparator(yul::Dialect const& _dialect): m_dialect(_dialect) {}

ASTComparator::ComparisonResult ASTComparator::compareObjects(yul::Object const& _a, yul::Object const& _b)
{
	m_mismatch.reset();
	if (compare(_a, _b))
		return ComparisonResult::equivalent();
	return ComparisonResult::failure(std::move(*m_mismatch));
}

bool ASTComparator::compare(yul::Object const& _a, yul::Object const& _b)
{
	Path objectPath(*this, fmt::format(R"(Object("{}"/"{}"))", _a.name, _b.name));

	if (_a.subObjects.size() != _b.subObjects.size())
		return fail(fmt::format("different number of sub-objects ({} vs {})", _a.subObjects.size(), _b.subObjects.size()));

	if (_a.hasCode() != _b.hasCode())
		return fail("one has code, the other does not");

	if (_a.hasCode())
	{
		ScopeBimap::Scope scope(m_bimap);
		Path codePath(*this, "code");
		if (!compare(_a.code()->root(), _b.code()->root()))
			return false;
	}

	for (size_t i = 0; i < _a.subObjects.size(); ++i)
	{
		auto const& subA = *_a.subObjects[i];
		auto const& subB = *_b.subObjects[i];

		if (typeid(subA) != typeid(subB))
			return fail(fmt::format("sub-object[{}]: type mismatch", i));

		if (auto const* objA = dynamic_cast<yul::Object const*>(&subA))
		{
			if (!compare(*objA, dynamic_cast<yul::Object const&>(subB)))
				return false;
		}
	}
	return true;
}

ASTComparator::Path::Path(ASTComparator& _comparator, std::string _segment): m_comparator(_comparator)
{
	m_comparator.m_pathStack.push_back(std::move(_segment));
}

ASTComparator::Path::~Path()
{
	m_comparator.m_pathStack.pop_back();
}

std::string ASTComparator::currentPath() const
{
	return fmt::format("{}", fmt::join(m_pathStack, " > "));
}

bool ASTComparator::fail(std::string _reason)
{
	m_mismatch = Mismatch{currentPath(), std::move(_reason), {}, {}};
	return false;
}

bool ASTComparator::compare(yul::Block const& _a, yul::Block const& _b)
{
	ScopeBimap::Scope blockScope(m_bimap);
	if (_a.statements.size() != _b.statements.size())
		return fail(fmt::format("block statement count differs ({} vs {})", _a.statements.size(), _b.statements.size()));
	for (size_t i = 0; i < _a.statements.size(); ++i)
	{
		Path statementPath(*this, fmt::format("stmt[{}]", i));
		if (!compare(_a.statements[i], _b.statements[i]))
			return false;
	}
	return true;
}

bool ASTComparator::compare(yul::ExpressionStatement const& _a, yul::ExpressionStatement const& _b)
{
	return compare(_a.expression, _b.expression);
}

bool ASTComparator::compare(yul::Assignment const& _a, yul::Assignment const& _b)
{
	Path statementPath(*this, "Assignment");
	if (_a.variableNames.size() != _b.variableNames.size())
		return fail("assignment target count differs", _a, _b);
	for (size_t i = 0; i < _a.variableNames.size(); ++i)
		if (!compare(_a.variableNames[i], _b.variableNames[i]))
		{
			return fail(fmt::format("variable name {} differs", i), _a, _b);
		}
	if (static_cast<bool>(_a.value) != static_cast<bool>(_b.value))
		return fail("assignment value presence differs", _a, _b);
	if (_a.value && !compare(*_a.value, *_b.value))
		return false;
	return true;
}

bool ASTComparator::compare(yul::VariableDeclaration const& _a, yul::VariableDeclaration const& _b)
{
	Path path(*this, "VariableDeclaration");
	if (_a.variables.size() != _b.variables.size())
		return fail("variable declaration count differs", _a, _b);
	for (size_t i = 0; i < _a.variables.size(); ++i)
		if (!m_bimap.tryMap(_a.variables[i].name, _b.variables[i].name))
			return fail(fmt::format(R"(variable name mapping inconsistent: "{}" vs "{}")",
				_a.variables[i].name.str(), _b.variables[i].name.str()), _a, _b);
	if (static_cast<bool>(_a.value) != static_cast<bool>(_b.value))
		return fail("variable declaration value presence differs", _a, _b);
	if (_a.value && !compare(*_a.value, *_b.value))
		return false;
	return true;
}

bool ASTComparator::compare(yul::FunctionDefinition const& _a, yul::FunctionDefinition const& _b)
{
	Path path(*this, fmt::format(R"(FunctionDefinition("{}"/"{}"))", _a.name.str(), _b.name.str()));
	if (!m_bimap.tryMap(_a.name, _b.name))
		return fail("function name mapping inconsistent");
	if (_a.parameters.size() != _b.parameters.size())
		return fail(fmt::format("parameter count differs ({} vs {})", _a.parameters.size(), _b.parameters.size()));
	if (_a.returnVariables.size() != _b.returnVariables.size())
		return fail(fmt::format("return variable count differs ({} vs {})", _a.returnVariables.size(), _b.returnVariables.size()));
	{
		ScopeBimap::Scope scope(m_bimap);
		for (size_t i = 0; i < _a.parameters.size(); ++i)
			if (!m_bimap.tryMap(_a.parameters[i].name, _b.parameters[i].name))
				return fail("parameter name mapping inconsistent");
		for (size_t i = 0; i < _a.returnVariables.size(); ++i)
			if (!m_bimap.tryMap(_a.returnVariables[i].name, _b.returnVariables[i].name))
				return fail("return variable name mapping inconsistent");
		if (!compare(_a.body, _b.body))
			return false;
	}
	return true;
}

bool ASTComparator::compare(yul::If const& _a, yul::If const& _b)
{
	Path path(*this, "If");
	if (!compare(*_a.condition, *_b.condition))
		return false;
	if (!compare(_a.body, _b.body))
		return false;
	return true;
}

bool ASTComparator::compare(yul::Switch const& _a, yul::Switch const& _b)
{
	Path path(*this, "Switch");
	if (!compare(*_a.expression, *_b.expression))
		return false;
	if (_a.cases.size() != _b.cases.size())
		return fail(fmt::format("case count differs ({} vs {})", _a.cases.size(), _b.cases.size()));
	for (size_t i = 0; i < _a.cases.size(); ++i)
	{
		Path casePath(*this, fmt::format("case[{}]", i));
		if (static_cast<bool>(_a.cases[i].value) != static_cast<bool>(_b.cases[i].value))
			return fail("case value presence differs (default vs non-default)");
		if (_a.cases[i].value)
			if (!compare(*_a.cases[i].value, *_b.cases[i].value))
				return false;
		if (!compare(_a.cases[i].body, _b.cases[i].body))
			return false;
	}
	return true;
}

bool ASTComparator::compare(yul::ForLoop const& _a, yul::ForLoop const& _b)
{
	Path path(*this, "ForLoop");
	ScopeBimap::Scope scope(m_bimap);
	return
		compare(_a.pre, _b.pre) &&
		compare(*_a.condition, *_b.condition) &&
		compare(_a.post, _b.post) &&
		compare(_a.body, _b.body);
}

bool ASTComparator::compare(yul::Break const&, yul::Break const&)
{
	return true;
}

bool ASTComparator::compare(yul::Continue const&, yul::Continue const&)
{
	return true;
}

bool ASTComparator::compare(yul::Leave const&, yul::Leave const&)
{
	return true;
}

bool ASTComparator::compare(yul::BuiltinName const& _a, yul::BuiltinName const& _b)
{
	if (_a.handle != _b.handle)
		return fail(fmt::format(R"(builtin function mismatch: "{}" vs "{}")",
			m_dialect.builtin(_a.handle).name, m_dialect.builtin(_b.handle).name));
	return true;
}

bool ASTComparator::compare(yul::FunctionCall const& _a, yul::FunctionCall const& _b)
{
	if (!compare(_a.functionName, _b.functionName))
		return false;
	if (_a.arguments.size() != _b.arguments.size())
		return fail("argument count differs", _a, _b);
	for (size_t i = 0; i < _a.arguments.size(); ++i)
	{
		Path path(*this, fmt::format("arg[{}]", i));
		if (!compare(_a.arguments[i], _b.arguments[i]))
			return false;
	}
	return true;
}

bool ASTComparator::compare(yul::Identifier const& _a, yul::Identifier const& _b)
{
	if (!m_bimap.tryMap(_a.name, _b.name))
		return fail(fmt::format(R"(identifier mapping inconsistent: "{}" vs "{}")", _a.name.str(), _b.name.str()));
	return true;
}

bool ASTComparator::compare(yul::Literal const& _a, yul::Literal const& _b)
{
	if (_a.kind != _b.kind)
		return fail("literal kind mismatch");
	if (_a.value != _b.value)
		return fail("literal value mismatch");
	return true;
}
