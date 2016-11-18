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
/** @file ConstantOptimiser.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 */

#pragma once

#include <vector>
#include <libdevcore/CommonData.h>
#include <libdevcore/CommonIO.h>

namespace dev
{
namespace eth
{

class AssemblyItem;
using AssemblyItems = std::vector<AssemblyItem>;
class Assembly;

/**
 * Abstract base class for one way to change how constants are represented in the code.
 */
class ConstantOptimisationMethod
{
public:
	/// Tries to optimised how constants are represented in the source code and modifies
	/// @a _assembly and its @a _items.
	/// @returns zero if no optimisations could be performed.
	static unsigned optimiseConstants(
		bool _isCreation,
		size_t _runs,
		Assembly& _assembly,
		AssemblyItems& _items
	);

	struct Params
	{
		bool isCreation; ///< Whether this is called during contract creation or runtime.
		size_t runs; ///< Estimated number of calls per opcode oven the lifetime of the contract.
		size_t multiplicity; ///< Number of times the constant appears in the code.
	};

	explicit ConstantOptimisationMethod(Params const& _params, u256 const& _value):
		m_params(_params), m_value(_value) {}
	virtual bigint gasNeeded() = 0;
	virtual void execute(Assembly& _assembly, AssemblyItems& _items) = 0;

protected:
	size_t dataSize() const { return std::max<size_t>(1, dev::bytesRequired(m_value)); }

	/// @returns the run gas for the given items ignoring special gas costs
	static bigint simpleRunGas(AssemblyItems const& _items);
	/// @returns the gas needed to store the given data literally
	bigint dataGas(bytes const& _data) const;
	/// @returns the gas needed to store the value literally
	bigint dataGas() const { return dataGas(toCompactBigEndian(m_value, 1)); }
	static size_t bytesRequired(AssemblyItems const& _items);
	/// @returns the combined estimated gas usage taking @a m_params into account.
	bigint combineGas(
		bigint const& _runGas,
		bigint const& _repeatedDataGas,
		bigint const& _uniqueDataGas
	)
	{
		// _runGas is not multiplied by _multiplicity because the runs are "per opcode"
		return m_params.runs * _runGas + m_params.multiplicity * _repeatedDataGas + _uniqueDataGas;
	}

	/// Replaces the constant by the code given in @a _replacement.
	void replaceConstants(AssemblyItems& _items, AssemblyItems const& _replacement) const;

	Params m_params;
	u256 const& m_value;
};

/**
 * Optimisation method that pushes the constant to the stack literally. This is the default method,
 * i.e. executing it does not alter the Assembly.
 */
class LiteralMethod: public ConstantOptimisationMethod
{
public:
	explicit LiteralMethod(Params const& _params, u256 const& _value):
		ConstantOptimisationMethod(_params, _value) {}
	virtual bigint gasNeeded() override;
	virtual void execute(Assembly&, AssemblyItems&) override {}
};

/**
 * Method that stores the data in the .data section of the code and copies it to the stack.
 */
class CodeCopyMethod: public ConstantOptimisationMethod
{
public:
	explicit CodeCopyMethod(Params const& _params, u256 const& _value);
	virtual bigint gasNeeded() override;
	virtual void execute(Assembly& _assembly, AssemblyItems& _items) override;

protected:
	AssemblyItems m_copyRoutine;
};

/**
 * Method that tries to compute the constant.
 */
class ComputeMethod: public ConstantOptimisationMethod
{
public:
	explicit ComputeMethod(Params const& _params, u256 const& _value):
		ConstantOptimisationMethod(_params, _value)
	{
		m_routine = findRepresentation(m_value);
	}

	virtual bigint gasNeeded() override { return gasNeeded(m_routine); }
	virtual void execute(Assembly&, AssemblyItems& _items) override
	{
		replaceConstants(_items, m_routine);
	}

protected:
	/// Tries to recursively find a way to compute @a _value.
	AssemblyItems findRepresentation(u256 const& _value);
	bigint gasNeeded(AssemblyItems const& _routine);

	AssemblyItems m_routine;
};

}
}
