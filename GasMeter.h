/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file GasMeter.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 */

#pragma once

#include <ostream>
#include <tuple>
#include <libevmasm/ExpressionClasses.h>
#include <libevmasm/AssemblyItem.h>

namespace dev
{
namespace eth
{

class KnownState;

/**
 * Class that helps computing the maximum gas consumption for instructions.
 * Has to be initialized with a certain known state that will be automatically updated for
 * each call to estimateMax. These calls have to supply strictly subsequent AssemblyItems.
 * A new gas meter has to be constructed (with a new state) for control flow changes.
 */
class GasMeter
{
public:
	struct GasConsumption
	{
		GasConsumption(u256 _value = 0, bool _infinite = false): value(_value), isInfinite(_infinite) {}
		static GasConsumption infinite() { return GasConsumption(0, true); }

		GasConsumption& operator+=(GasConsumption const& _other);
		bool operator<(GasConsumption const& _other) const { return this->tuple() < _other.tuple(); }

		std::tuple<bool const&, u256 const&> tuple() const { return std::tie(isInfinite, value); }

		u256 value;
		bool isInfinite;
	};

	/// Constructs a new gas meter given the current state.
	explicit GasMeter(std::shared_ptr<KnownState> const& _state, u256 const& _largestMemoryAccess = 0):
		m_state(_state), m_largestMemoryAccess(_largestMemoryAccess) {}

	/// @returns an upper bound on the gas consumed by the given instruction and updates
	/// the state.
	GasConsumption estimateMax(AssemblyItem const& _item);

	u256 const& largestMemoryAccess() const { return m_largestMemoryAccess; }

private:
	/// @returns _multiplier * (_value + 31) / 32, if _value is a known constant and infinite otherwise.
	GasConsumption wordGas(u256 const& _multiplier, ExpressionClasses::Id _value);
	/// @returns the gas needed to access the given memory position.
	/// @todo this assumes that memory was never accessed before and thus over-estimates gas usage.
	GasConsumption memoryGas(ExpressionClasses::Id _position);
	/// @returns the memory gas for accessing the memory at a specific offset for a number of bytes
	/// given as values on the stack at the given relative positions.
	GasConsumption memoryGas(int _stackPosOffset, int _stackPosSize);

	static GasConsumption runGas(Instruction _instruction);

	std::shared_ptr<KnownState> m_state;
	/// Largest point where memory was accessed since the creation of this object.
	u256 m_largestMemoryAccess;
};

inline std::ostream& operator<<(std::ostream& _str, GasMeter::GasConsumption const& _consumption)
{
	if (_consumption.isInfinite)
		return _str << "inf";
	else
		return _str << std::dec << _consumption.value;
}


}
}
