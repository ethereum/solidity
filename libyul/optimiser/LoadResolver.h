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
 * Optimisation stage that replaces expressions of type ``sload(x)`` by the value
 * currently stored in storage, if known.
 */

#pragma once

#include <libyul/optimiser/DataFlowAnalyzer.h>
#include <libyul/optimiser/OptimiserStep.h>

namespace solidity::yul
{

/**
 * Optimisation stage that replaces expressions of type ``sload(x)`` and ``mload(x)`` by the value
 * currently stored in storage resp. memory, if known.
 *
 * Also evaluates simple ``keccak256(a, c)`` when the value at memory location `a` is known and `c`
 * is a constant `<= 32`.
 *
 * Works best if the code is in SSA form.
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter.
 */
class LoadResolver: public DataFlowAnalyzer
{
public:
	static constexpr char const* name{"LoadResolver"};
	/// Run the load resolver on the given complete AST.
	static void run(OptimiserStepContext&, Block& _ast);

private:
	LoadResolver(
		Dialect const& _dialect,
		std::map<YulString, SideEffects> _functionSideEffects,
		bool _containsMSize,
		std::optional<size_t> _expectedExecutionsPerDeployment
	):
		DataFlowAnalyzer(_dialect, MemoryAndStorage::Analyze, std::move(_functionSideEffects)),
		m_containsMSize(_containsMSize),
		m_expectedExecutionsPerDeployment(std::move(_expectedExecutionsPerDeployment))
	{}

protected:
	using ASTModifier::visit;
	void visit(Expression& _e) override;

	void tryResolve(
		Expression& _e,
		StoreLoadLocation _location,
		std::vector<Expression> const& _arguments
	);

	/// Evaluates simple ``keccak256(a, c)`` when the value at memory location ``a`` is known and
	/// `c` is a constant `<= 32`.
	void tryEvaluateKeccak(
		Expression& _e,
		std::vector<Expression> const& _arguments
	);

	/// If the AST contains `msize`, then we skip resolving `mload` and `keccak256`.
	bool m_containsMSize = false;
	/// The --optimize-runs parameter. Value `nullopt` represents creation code.
	std::optional<size_t> m_expectedExecutionsPerDeployment;
};

}
