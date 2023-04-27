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
using namespace std;

namespace solidity::evmasm
{

bool EVMAssemblyStack::parseAndAnalyze(string const& _sourceName, string const& _source)
{
	solAssert(!m_evmAssembly);

	m_name = _sourceName;
	if (!jsonParseStrict(_source, m_json))
		return false;

	auto result = evmasm::Assembly::fromJSON(m_json);
	m_evmAssembly = result.first;
	m_sourceList = result.second;

	return m_evmAssembly != nullptr;
}

void EVMAssemblyStack::assemble()
{
	solAssert(m_evmAssembly->isCreation());
	solAssert(m_evmAssembly);
	solAssert(!m_evmRuntimeAssembly);

	m_object = m_evmAssembly->assemble();
	m_sourceMapping.emplace(AssemblyItem::computeSourceMapping(m_evmAssembly->items(), sourceIndices()));
	if (m_evmAssembly->numSubs() > 0)
	{
		m_evmRuntimeAssembly = make_shared<evmasm::Assembly>(m_evmAssembly->sub(0));
		solAssert(m_evmRuntimeAssembly && !m_evmRuntimeAssembly->isCreation());
		m_runtimeSourceMapping.emplace(AssemblyItem::computeSourceMapping(m_evmRuntimeAssembly->items(), sourceIndices()));
		m_runtimeObject = m_evmRuntimeAssembly->assemble();
	}
}


LinkerObject const& EVMAssemblyStack::object(string const& _contractName) const
{
	solAssert(_contractName == m_name);
	return m_object;
}

LinkerObject const& EVMAssemblyStack::runtimeObject(string const& _contractName) const
{
	solAssert(_contractName == m_name);
	return m_runtimeObject;
}

map<string, unsigned> EVMAssemblyStack::sourceIndices() const
{
	solAssert(m_evmAssembly);
	map<string, unsigned> indices;
	unsigned index = 0;
	for (auto const& s: m_sourceList)
		if (s != CompilerContext::yulUtilityFileName())
			indices[s] = index++;

	if (indices.find(CompilerContext::yulUtilityFileName()) == indices.end())
		indices[CompilerContext::yulUtilityFileName()] = index++;
	return indices;
}

string const* EVMAssemblyStack::sourceMapping(string const& _contractName) const
{
	solAssert(_contractName == m_name);
	return m_sourceMapping.has_value() ? &m_sourceMapping.value() : nullptr;
}

string const* EVMAssemblyStack::runtimeSourceMapping(string const& _contractName) const
{
	solAssert(_contractName == m_name);
	return m_runtimeSourceMapping.has_value() ? &m_runtimeSourceMapping.value() : nullptr;
}

Json::Value EVMAssemblyStack::assemblyJSON(string const& _contractName) const
{
	solAssert(_contractName == m_name);
	solAssert(m_evmAssembly);
	vector<string> sources = sourceNames();
	if (find(sources.begin(), sources.end(), CompilerContext::yulUtilityFileName()) == sources.end())
		sources.emplace_back(CompilerContext::yulUtilityFileName());
	return m_evmAssembly->assemblyJSON(sources);
}

string EVMAssemblyStack::assemblyString(string const& _contractName, StringMap const& _sourceCodes) const
{
	solAssert(_contractName == m_name);
	solAssert(m_evmAssembly);
	return m_evmAssembly->assemblyString(m_debugInfoSelection, _sourceCodes);
}

string const EVMAssemblyStack::filesystemFriendlyName(string const& _contractName) const
{
	solAssert(_contractName == m_name);
	return m_name;
}

vector<string> EVMAssemblyStack::sourceNames() const
{
	return m_sourceList;
}

} // namespace solidity::evmasm
