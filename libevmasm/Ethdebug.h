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

#include <libsolutil/JSON.h>

#include <libevmasm/Assembly.h>
#include <libevmasm/EthdebugSchema.h>
#include <libevmasm/LinkerObject.h>

#include <map>

namespace solidity::evmasm::ethdebug
{

struct Source
{
	unsigned id;
	std::string path;
	std::string contents;
	std::string language;
};

// returns ethdebug/format/program.
Json program(std::string_view _name, unsigned _sourceID, Assembly const& _assembly, LinkerObject const& _linkerObject);

// returns ethdebug/format/info/resources
Json resources(
	std::vector<Source> const& _sources,
	std::string_view _version,
	std::map<std::string, schema::type::Type> _types = {},
	std::map<std::string, schema::pointer::Template> _pointers = {}
);

// returns the 'compilation' object from ethdebug/format/info/resources
Json compilation(std::vector<Source> const& _sources, std::string_view _version);

} // namespace solidity::evmasm::ethdebug
