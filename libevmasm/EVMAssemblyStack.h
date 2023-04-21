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

#include <libevmasm/Assembly.h>
#include <libevmasm/LinkerObject.h>

#include <libsolutil/JSON.h>

#include <map>
#include <string>

namespace solidity::evmasm
{

class EVMAssemblyStack
{
public:
	explicit EVMAssemblyStack(langutil::EVMVersion _evmVersion): m_evmVersion(_evmVersion) {}

	/// Runs parsing and analysis, returns false if input cannot be assembled.
	/// Multiple calls overwrite the previous state.
	bool parseAndAnalyze(std::string const& _sourceName, std::string const& _source);

	void assemble();

	std::string const& name() const { return m_name; }

	evmasm::LinkerObject const& object() const { return m_object; }
	evmasm::LinkerObject const& runtimeObject() const { return m_runtimeObject; }

	std::shared_ptr<evmasm::Assembly> const& evmAssembly() const { return m_evmAssembly; }
	std::shared_ptr<evmasm::Assembly> const& evmRuntimeAssembly() const { return m_evmRuntimeAssembly; }


private:
	langutil::EVMVersion m_evmVersion;
	std::string m_name;
	Json::Value m_json;
	std::shared_ptr<evmasm::Assembly> m_evmAssembly;
	std::shared_ptr<evmasm::Assembly> m_evmRuntimeAssembly;
	evmasm::LinkerObject m_object; ///< Deployment object (includes the runtime sub-object).
	evmasm::LinkerObject m_runtimeObject; ///< Runtime object.
};

} // namespace solidity::evmasm
