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

#include <utility>

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

	m_evmAssembly = evmasm::Assembly::fromJSON(m_json);
	return m_evmAssembly != nullptr;
}

void EVMAssemblyStack::assemble()
{
	solAssert(m_evmAssembly->isCreation());
	solAssert(m_evmAssembly);
	solAssert(!m_evmRuntimeAssembly);

	m_object = m_evmAssembly->assemble();
	if (m_evmAssembly->numSubs() > 0)
	{
		m_evmRuntimeAssembly = make_shared<evmasm::Assembly>(m_evmAssembly->sub(0));
		solAssert(m_evmRuntimeAssembly && !m_evmRuntimeAssembly->isCreation());
		m_runtimeObject = m_evmRuntimeAssembly->assemble();
	}
}

} // namespace solidity::evmasm
