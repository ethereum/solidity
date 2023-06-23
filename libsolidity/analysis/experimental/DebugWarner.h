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

#include <libsolidity/ast/ASTVisitor.h>

#include <liblangutil/ErrorReporter.h>

namespace solidity::frontend::experimental
{
class Analysis;

class DebugWarner: public ASTConstVisitor
{
public:
	DebugWarner(Analysis& _analysis);

	bool analyze(ASTNode const& _astRoot);

private:
	bool visitNode(ASTNode const& _node) override;

	Analysis& m_analysis;
	langutil::ErrorReporter& m_errorReporter;
};

}
