// SPDX-License-Identifier: GPL-3.0
/**
 * Yul code and data object container.
 */

#include <libyul/Object.h>

#include <libyul/AsmPrinter.h>
#include <libyul/Exceptions.h>

#include <libsolutil/Visitor.h>
#include <libsolutil/CommonData.h>

#include <boost/algorithm/string/replace.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

namespace
{

string indent(std::string const& _input)
{
	if (_input.empty())
		return _input;
	return boost::replace_all_copy("    " + _input, "\n", "\n    ");
}

}

string Data::toString(Dialect const*) const
{
	return "data \"" + name.str() + "\" hex\"" + util::toHex(data) + "\"";
}

string Object::toString(Dialect const* _dialect) const
{
	yulAssert(code, "No code");
	string inner = "code " + (_dialect ? AsmPrinter{*_dialect} : AsmPrinter{})(*code);

	for (auto const& obj: subObjects)
		inner += "\n" + obj->toString(_dialect);

	return "object \"" + name.str() + "\" {\n" + indent(inner) + "\n}";
}

set<YulString> Object::dataNames() const
{
	set<YulString> names;
	names.insert(name);
	for (auto const& subObject: subIndexByName)
		names.insert(subObject.first);
	// The empty name is not valid
	names.erase(YulString{});
	return names;
}
