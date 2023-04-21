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

#include <libevmasm/LinkerObject.h>

#include <libsolutil/Common.h>
#include <libsolutil/JSON.h>

#include <string>
#include <vector>

namespace solidity::evmasm
{

class AbstractAssemblyStack
{
public:
	virtual ~AbstractAssemblyStack() {}

	virtual LinkerObject const& object(std::string const& _contractName) const = 0;
	virtual LinkerObject const& runtimeObject(std::string const& _contractName) const = 0;

	virtual std::string const* sourceMapping(std::string const& _contractName) const = 0;
	virtual std::string const* runtimeSourceMapping(std::string const& _contractName) const = 0;

	virtual Json::Value assemblyJSON(std::string const& _contractName) const = 0;
	virtual std::string assemblyString(std::string const& _contractName, StringMap const& _sourceCodes) const = 0;

	virtual std::string const filesystemFriendlyName(std::string const& _contractName) const = 0;

	virtual std::vector<std::string> contractNames() const = 0;
	virtual std::vector<std::string> sourceNames() const = 0;

	virtual bool compilationSuccessful() const = 0;
};

} // namespace solidity::evmasm
