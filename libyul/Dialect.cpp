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
 * Yul dialect.
 */

#include <libyul/Dialect.h>

#include <map>

using namespace yul;
using namespace std;

namespace
{

void addFunction(
	map<YulString, BuiltinFunction>& _repository,
	string const& _name,
	size_t _params,
	size_t _returns,
	bool _movable
)
{
	_repository[YulString{_name}] = BuiltinFunction{
		YulString{_name},
		vector<Type>(_params),
		vector<Type>(_returns),
		_movable
	};
}

class GenericBuiltins: public Builtins
{
public:
	GenericBuiltins(map<YulString, BuiltinFunction> const& _functions): m_functions(_functions) {}
	BuiltinFunction const* query(YulString _name) const
	{
		auto it = m_functions.find(_name);
		if (it != end(m_functions))
			return &it->second;
		else
			return nullptr;
	}
private:
	map<YulString, BuiltinFunction> const& m_functions;
};

}

Dialect Dialect::strictAssemblyForEVMObjects()
{
	static map<YulString, BuiltinFunction> functions;
	if (functions.empty())
	{
		addFunction(functions, "datasize", 1, 1, true);
		addFunction(functions, "dataoffset", 1, 1, true);
		addFunction(functions, "datacopy", 3, 0, false);
	}
	// The EVM instructions will be moved to builtins at some point.
	return Dialect{AsmFlavour::Strict, std::make_shared<GenericBuiltins>(functions)};
}
