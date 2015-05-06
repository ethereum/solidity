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
#include <libevmasm/AssemblyItem.h>

namespace dev
{
namespace eth
{

/**
 * Class that helps computing the maximum gas consumption for instructions.
 */
class GasMeter
{
public:
	struct GasConsumption
	{
		GasConsumption(u256 _value = 0, bool _infinite = false): value(_value), isInfinite(_infinite) {}
		static GasConsumption infinite() { return GasConsumption(0, true); }

		GasConsumption& operator+=(GasConsumption const& _otherS);
		std::ostream& operator<<(std::ostream& _str) const;

		u256 value;
		bool isInfinite;
	};

	/// Returns an upper bound on the gas consumed by the given instruction.
	GasConsumption estimateMax(AssemblyItem const& _item);

private:
	static GasConsumption runGas(Instruction _instruction);
};

inline std::ostream& operator<<(std::ostream& _str, GasMeter::GasConsumption const& _consumption)
{
	if (_consumption.isInfinite)
		return _str << "inf";
	else
		return _str << _consumption.value;
}


}
}
