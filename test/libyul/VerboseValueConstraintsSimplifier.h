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
 * Debugging / testing component that adds reporting to the value constraint based
 * optimiser stage.
 */

#pragma once

#include <libyul/optimiser/ValueConstraintBasedSimplifier.h>

namespace yul
{
namespace test
{

/**
 * Debugging / testing component that adds reporting to the value constraint based
 * optimiser stage.
 */
class VerboseValueConstraintsSimplifier: public ValueConstraintBasedSimplifier
{
public:
	static void run(Dialect const& _dialect, Block& _ast, std::string& _report);

protected:
	VerboseValueConstraintsSimplifier(Dialect const& _dialect, std::string& _report):
		ValueConstraintBasedSimplifier(_dialect),
		m_report(_report)
	{}
	void handleAssignment(std::set<YulString> const& _names, Expression* _value) override;

private:
	std::string& m_report;
};

}
}
