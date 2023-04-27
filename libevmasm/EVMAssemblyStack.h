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

#include <libevmasm/AbstractAssemblyStack.h>
#include <libevmasm/Assembly.h>
#include <libevmasm/LinkerObject.h>

#include <libsolutil/JSON.h>

#include <map>
#include <string>

namespace solidity::evmasm
{

class EVMAssemblyStack: public AbstractAssemblyStack
{
public:
	explicit EVMAssemblyStack(langutil::EVMVersion _evmVersion): m_evmVersion(_evmVersion) {}

	bool parseAndAnalyze(std::string const& _sourceName, std::string const& _source);

	void assemble();

	std::string const& name() const { return m_name; }

	virtual LinkerObject const& object(std::string const& _contractName) const override;
	virtual LinkerObject const& runtimeObject(std::string const& _contractName) const override;

	std::shared_ptr<evmasm::Assembly> const& evmAssembly() const { return m_evmAssembly; }
	std::shared_ptr<evmasm::Assembly> const& evmRuntimeAssembly() const { return m_evmRuntimeAssembly; }

	virtual std::string const* sourceMapping(std::string const& _contractName) const override;
	virtual std::string const* runtimeSourceMapping(std::string const& _contractName) const override;

	virtual Json::Value assemblyJSON(std::string const& _contractName) const override;
	virtual std::string assemblyString(std::string const& _contractName, StringMap const& _sourceCodes) const override;

	virtual std::string const filesystemFriendlyName(std::string const& _contractName) const override;

	virtual std::vector<std::string> contractNames() const override { return {m_name}; }
	virtual std::vector<std::string> sourceNames() const override;
	std::map<std::string, unsigned> sourceIndices() const;

	virtual bool compilationSuccessful() const override { return m_evmAssembly != nullptr; }

	void selectDebugInfo(langutil::DebugInfoSelection _debugInfoSelection)
	{
		m_debugInfoSelection = _debugInfoSelection;
	}

private:
	langutil::EVMVersion m_evmVersion;
	std::string m_name;
	Json::Value m_json;
	std::shared_ptr<evmasm::Assembly> m_evmAssembly;
	std::shared_ptr<evmasm::Assembly> m_evmRuntimeAssembly;
	evmasm::LinkerObject m_object; ///< Deployment object (includes the runtime sub-object).
	evmasm::LinkerObject m_runtimeObject; ///< Runtime object.
	std::vector<std::string> m_sourceList;
	langutil::DebugInfoSelection m_debugInfoSelection = langutil::DebugInfoSelection::Default();
	std::optional<std::string const> m_sourceMapping;
	std::optional<std::string const> m_runtimeSourceMapping;
};

} // namespace solidity::evmasm
