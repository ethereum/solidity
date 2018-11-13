/*(
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
 * Component that can compare ASTs for equality on a syntactic basis.
 */

#include <libyul/optimiser/SyntacticalEquality.h>

#include <libyul/Exceptions.h>

#include <libsolidity/inlineasm/AsmData.h>

#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace dev::yul;

bool SyntacticalEqualityChecker::equal(Expression const& _e1, Expression const& _e2)
{
	if (_e1.type() != _e2.type())
		return false;
	// TODO This somehow calls strcmp - WHERE?

	// TODO This should be replaced by some kind of AST walker as soon as it gets
	// more complex.
	if (_e1.type() == typeid(FunctionalInstruction))
	{
		auto const& e1 = boost::get<FunctionalInstruction>(_e1);
		auto const& e2 = boost::get<FunctionalInstruction>(_e2);
		return
			e1.instruction == e2.instruction &&
			equalVector(e1.arguments, e2.arguments);
	}
	else if (_e1.type() == typeid(FunctionCall))
	{
		auto const& e1 = boost::get<FunctionCall>(_e1);
		auto const& e2 = boost::get<FunctionCall>(_e2);
		return
			equal(e1.functionName, e2.functionName) &&
			equalVector(e1.arguments, e2.arguments);
	}
	else if (_e1.type() == typeid(Identifier))
		return boost::get<Identifier>(_e1).name == boost::get<Identifier>(_e2).name;
	else if (_e1.type() == typeid(Literal))
	{
		auto const& e1 = boost::get<Literal>(_e1);
		auto const& e2 = boost::get<Literal>(_e2);
		return e1.kind == e2.kind && e1.value == e2.value && e1.type == e2.type;
	}
	else
	{
		assertThrow(false, OptimizerException, "Invalid expression");
	}
	return false;
}

bool SyntacticalEqualityChecker::equalVector(vector<Expression> const& _e1, vector<Expression> const& _e2)
{
	return _e1.size() == _e2.size() &&
		std::equal(begin(_e1), end(_e1), begin(_e2), SyntacticalEqualityChecker::equal);

}
