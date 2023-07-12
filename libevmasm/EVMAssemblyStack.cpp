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

#include <libevmasm/EVMAssemblyStack.h>

#include <libsolutil/JSON.h>
#include <liblangutil/Exceptions.h>
#include <libsolidity/codegen/CompilerContext.h>

using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::frontend;

namespace solidity::evmasm
{

void EVMAssemblyStack::parseAndAnalyze(std::string const& _sourceName, std::string const& _source)
{
	solAssert(!m_evmAssembly);
	m_name = _sourceName;
	solRequire(jsonParseStrict(_source, m_json), AssemblyImportException, "Could not parse JSON file.");
	auto result = evmasm::Assembly::fromJSON(m_json);
	m_evmAssembly = result.first;
	m_sourceList = result.second;
	solRequire(m_evmAssembly != nullptr, AssemblyImportException, "Could not create evm assembly object.");
}

void EVMAssemblyStack::assemble()
{
	solAssert(m_evmAssembly);
	solAssert(m_evmAssembly->isCreation());
	solAssert(!m_evmRuntimeAssembly);

	m_object = m_evmAssembly->assemble();
	m_sourceMapping = AssemblyItem::computeSourceMapping(m_evmAssembly->items(), sourceIndices());
	if (m_evmAssembly->numSubs() > 0)
	{
		m_evmRuntimeAssembly = std::make_shared<evmasm::Assembly>(m_evmAssembly->sub(0));
		solAssert(m_evmRuntimeAssembly && !m_evmRuntimeAssembly->isCreation());
		m_runtimeSourceMapping = AssemblyItem::computeSourceMapping(m_evmRuntimeAssembly->items(), sourceIndices());
		m_runtimeObject = m_evmRuntimeAssembly->assemble();
	}
}

LinkerObject const& EVMAssemblyStack::object(std::string const& _contractName) const
{
	solAssert(_contractName == m_name);
	return m_object;
}

LinkerObject const& EVMAssemblyStack::runtimeObject(std::string const& _contractName) const
{
	solAssert(_contractName == m_name);
	return m_runtimeObject;
}

std::map<std::string, unsigned> EVMAssemblyStack::sourceIndices() const
{
	solAssert(m_evmAssembly);
	std::map<std::string, unsigned> indices;
	unsigned index = 0;
	for (auto const& s: m_sourceList)
		if (s != CompilerContext::yulUtilityFileName())
			indices[s] = index++;

	if (indices.find(CompilerContext::yulUtilityFileName()) == indices.end())
		indices[CompilerContext::yulUtilityFileName()] = index++;
	return indices;
}

std::string const* EVMAssemblyStack::sourceMapping(std::string const& _contractName) const
{
	solAssert(_contractName == m_name);
	return &m_sourceMapping;
}

std::string const* EVMAssemblyStack::runtimeSourceMapping(std::string const& _contractName) const
{
	solAssert(_contractName == m_name);
	return &m_runtimeSourceMapping;
}

Json::Value EVMAssemblyStack::assemblyJSON(std::string const& _contractName) const
{
	solAssert(_contractName == m_name);
	solAssert(m_evmAssembly);
	std::vector<std::string> sources = sourceNames();
	if (find(sources.begin(), sources.end(), CompilerContext::yulUtilityFileName()) == sources.end())
		sources.emplace_back(CompilerContext::yulUtilityFileName());
	return m_evmAssembly->assemblyJSON(sources);
}

std::string EVMAssemblyStack::assemblyString(std::string const& _contractName, StringMap const& _sourceCodes) const
{
	solAssert(_contractName == m_name);
	solAssert(m_evmAssembly);
	return m_evmAssembly->assemblyString(m_debugInfoSelection, _sourceCodes);
}

std::string const EVMAssemblyStack::filesystemFriendlyName(std::string const& _contractName) const
{
	solAssert(_contractName == m_name);
	return m_name;
}

std::vector<std::string> EVMAssemblyStack::sourceNames() const
{
	return m_sourceList;
}

} // namespace solidity::evmasm
