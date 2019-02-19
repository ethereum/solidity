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

#pragma once

#include <liblangutil/EVMVersion.h>
#include <test/Common.h>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/version.hpp>

#include <functional>

namespace dev
{
namespace test
{

struct Options: CommonOptions
{
	bool showMessages = false;
	bool useABIEncoderV2 = false;

	solidity::EVMVersion evmVersion() const;

	static Options const& get();

private:
	std::string evmVersionString;

	Options();
};

}
}
