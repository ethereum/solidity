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
 * Optimisation stage that replaces constants by expressions that compute them.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/YulString.h>
#include <libyul/Dialect.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/AsmData.h>

#include <liblangutil/SourceLocation.h>

#include <libdevcore/Common.h>

#include <tuple>
#include <map>
#include <memory>

namespace yul
{
struct Dialect;
class GasMeter;

/**
 * Optimisation stage that replaces constants by expressions that compute them.
 *
 * Prerequisite: None
 */
class ConstantOptimiser: public ASTModifier
{
public:
	ConstantOptimiser(EVMDialect const& _dialect, GasMeter const& _meter):
		m_dialect(_dialect),
		m_meter(_meter)
	{}

	void visit(Expression& _e) override;

	struct Representation
	{
		std::unique_ptr<Expression> expression;
		size_t cost = size_t(-1);
	};

private:
	EVMDialect const& m_dialect;
	GasMeter const& m_meter;
	std::map<dev::u256, Representation> m_cache;
};

class RepresentationFinder
{
public:
	using Representation = ConstantOptimiser::Representation;
	RepresentationFinder(
		EVMDialect const& _dialect,
		GasMeter const& _meter,
		langutil::SourceLocation _location,
		std::map<dev::u256, Representation>& _cache
	):
		m_dialect(_dialect),
		m_meter(_meter),
		m_location(std::move(_location)),
		m_cache(_cache)
	{}

	/// @returns a cheaper representation for the number than its representation
	/// as a literal or nullptr otherwise.
	Expression const* tryFindRepresentation(dev::u256 const& _value);

private:
	/// Recursively try to find the cheapest representation of the given number,
	/// literal if necessary.
	Representation const& findRepresentation(dev::u256 const& _value);

	Representation represent(dev::u256 const& _value) const;
	Representation represent(YulString _instruction, Representation const& _arg) const;
	Representation represent(YulString _instruction, Representation const& _arg1, Representation const& _arg2) const;

	Representation min(Representation _a, Representation _b);

	EVMDialect const& m_dialect;
	GasMeter const& m_meter;
	langutil::SourceLocation m_location;
	/// Counter for the complexity of optimization, will stop when it reaches zero.
	size_t m_maxSteps = 10000;
	std::map<dev::u256, Representation>& m_cache;
};

}
