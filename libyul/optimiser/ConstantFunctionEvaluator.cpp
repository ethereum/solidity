
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
/**
 * Optimiser component that performs function inlining for arbitrary functions.
 */

#include <libyul/optimiser/ConstantFunctionEvaluator.h>

#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/FunctionCallFinder.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/Metrics.h>
#include <libyul/optimiser/SSAValueTracker.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/Exceptions.h>
#include <libyul/AST.h>
#include <libyul/Dialect.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Visitor.h>

#include <range/v3/action/remove.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/zip.hpp>

using namespace solidity;
using namespace solidity::yul;

void ConstantFunctionEvaluator::run(OptimiserStepContext& _context, Block& _ast)
{
	ConstantFunctionEvaluator cfe(_ast, _context.dialect);
}


ConstantFunctionEvaluator::ConstantFunctionEvaluator(Block& _ast, Dialect const& _dialect):
	m_ast(_ast),
	m_dialect(_dialect)
{
}


