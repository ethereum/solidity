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
/**
 * Yul code and data object container.
 */

#include <libyul/Object.h>

#include <libyul/AsmPrinter.h>
#include <libyul/Exceptions.h>

#include <libdevcore/Visitor.h>
#include <libdevcore/CommonData.h>

#include <boost/algorithm/string/replace.hpp>

using namespace dev;
using namespace yul;
using namespace std;

namespace
{

string indent(std::string const& _input)
{
	if (_input.empty())
		return _input;
	return boost::replace_all_copy("    " + _input, "\n", "\n    ");
}

}

string Data::toString(bool) const
{
	return "data \"" + name.str() + "\" hex\"" + dev::toHex(data) + "\"";
}

string Object::toString(bool _yul) const
{
	yulAssert(code, "No code");
	string inner = "code " + AsmPrinter{_yul}(*code);

	for (auto const& obj: subObjects)
		inner += "\n" + obj->toString(_yul);

	return "object \"" + name.str() + "\" {\n" + indent(inner) + "\n}";
}
