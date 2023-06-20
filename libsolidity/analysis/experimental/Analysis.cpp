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
#include <libsolidity/analysis/experimental/Analysis.h>

#include <libsolidity/analysis/experimental/SyntaxRestrictor.h>
#include <libsolidity/analysis/experimental/TypeInference.h>

using namespace std;
using namespace solidity::langutil;
using namespace solidity::frontend::experimental;

Analysis::Analysis(langutil::ErrorReporter& _errorReporter, uint64_t _maxAstId):
	m_errorReporter(_errorReporter),
	m_maxAstId(_maxAstId)
{
}

bool Analysis::check(vector<shared_ptr<SourceUnit const>> const& _sourceUnits)
{
	SyntaxRestrictor syntaxRestrictor{m_errorReporter};
	for (auto source: _sourceUnits)
		if (!syntaxRestrictor.check(*source))
			return false;
	TypeInference typeInference{*this};
	for (auto source: _sourceUnits)
		if (!typeInference.analyze(*source))
			return false;
	return true;
}
