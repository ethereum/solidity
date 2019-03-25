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

#include <test/libyul/VerboseValueConstraintsSimplifier.h>

#include <libdevcore/CommonData.h>
#include <libdevcore/StringUtils.h>

using namespace std;
using namespace dev;
using namespace yul;
using namespace yul::test;


void VerboseValueConstraintsSimplifier::run(Dialect const& _dialect, Block& _ast, string& _report)
{
	VerboseValueConstraintsSimplifier{_dialect, _report}(_ast);
}

void VerboseValueConstraintsSimplifier::handleAssignment(set<YulString> const& _names, Expression* _value)
{
	ValueConstraintBasedSimplifier::handleAssignment(_names, _value);

	for (YulString var: _names)
	{
		auto it = m_variableConstraints.find(var);
		if (it == m_variableConstraints.end())
			continue;
		m_report +=
			var.str() +
			":\n";
		ValueConstraint const& constr = it->second;
		if (boost::optional<u256> value = constr.isConstant())
			m_report += "       = " + formatNumberReadable(*value) + "\n";
		else
			m_report +=
				("    min: " + formatNumberReadable(it->second.minValue) + "\n") +
				("    max: " + formatNumberReadable(it->second.maxValue) + "\n") +
				("   minB: " + formatNumberReadable(it->second.minBits) + "\n") +
				("   maxB: " + formatNumberReadable(it->second.maxBits) + "\n");
	}
}
