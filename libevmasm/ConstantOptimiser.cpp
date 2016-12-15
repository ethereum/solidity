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

#include <libevmasm/ConstantOptimiser.h>
#include <libevmasm/Assembly.h>
#include <libevmasm/GasMeter.h>
using namespace std;
using namespace dev;
using namespace dev::eth;

unsigned ConstantOptimisationMethod::optimiseConstants(
	bool _isCreation,
	size_t _runs,
	Assembly& _assembly,
	AssemblyItems& _items
)
{
	unsigned optimisations = 0;
	map<AssemblyItem, size_t> pushes;
	for (AssemblyItem const& item: _items)
		if (item.type() == Push)
			pushes[item]++;
	for (auto it: pushes)
	{
		AssemblyItem const& item = it.first;
		if (item.data() < 0x100)
			continue;
		Params params;
		params.multiplicity = it.second;
		params.isCreation = _isCreation;
		params.runs = _runs;
		LiteralMethod lit(params, item.data());
		bigint literalGas = lit.gasNeeded();
		CodeCopyMethod copy(params, item.data());
		bigint copyGas = copy.gasNeeded();
		ComputeMethod compute(params, item.data());
		bigint computeGas = compute.gasNeeded();
		if (copyGas < literalGas && copyGas < computeGas)
		{
			copy.execute(_assembly, _items);
			optimisations++;
		}
		else if (computeGas < literalGas && computeGas < copyGas)
		{
			compute.execute(_assembly, _items);
			optimisations++;
		}
	}
	return optimisations;
}

bigint ConstantOptimisationMethod::simpleRunGas(AssemblyItems const& _items)
{
	bigint gas = 0;
	for (AssemblyItem const& item: _items)
		if (item.type() == Push)
			gas += GasMeter::runGas(Instruction::PUSH1);
		else if (item.type() == Operation)
			gas += GasMeter::runGas(item.instruction());
	return gas;
}

bigint ConstantOptimisationMethod::dataGas(bytes const& _data) const
{
	if (m_params.isCreation)
	{
		bigint gas;
		for (auto b: _data)
			gas += b ? GasCosts::txDataNonZeroGas : GasCosts::txDataZeroGas;
		return gas;
	}
	else
		return GasCosts::createDataGas * dataSize();
}

size_t ConstantOptimisationMethod::bytesRequired(AssemblyItems const& _items)
{
	size_t size = 0;
	for (AssemblyItem const& item: _items)
		size += item.bytesRequired(3); // assume 3 byte addresses
	return size;
}

void ConstantOptimisationMethod::replaceConstants(
	AssemblyItems& _items,
	AssemblyItems const& _replacement
) const
{
	assertThrow(_items.size() > 0, OptimizerException, "");
	for (size_t i = 0; i < _items.size(); ++i)
	{
		if (_items.at(i) != AssemblyItem(m_value))
			continue;
		_items[i] = _replacement[0];
		_items.insert(_items.begin() + i + 1, _replacement.begin() + 1, _replacement.end());
		i += _replacement.size() - 1;
	}
}

bigint LiteralMethod::gasNeeded()
{
	return combineGas(
		simpleRunGas({Instruction::PUSH1}),
		// PUSHX plus data
		(m_params.isCreation ? GasCosts::txDataNonZeroGas : GasCosts::createDataGas) + dataGas(),
		0
	);
}

CodeCopyMethod::CodeCopyMethod(Params const& _params, u256 const& _value):
	ConstantOptimisationMethod(_params, _value)
{
	m_copyRoutine = AssemblyItems{
		u256(0),
		Instruction::DUP1,
		Instruction::MLOAD, // back up memory
		u256(32),
		AssemblyItem(PushData, u256(1) << 16), // has to be replaced
		Instruction::DUP4,
		Instruction::CODECOPY,
		Instruction::DUP2,
		Instruction::MLOAD,
		Instruction::SWAP2,
		Instruction::MSTORE
	};
}

bigint CodeCopyMethod::gasNeeded()
{
	return combineGas(
		// Run gas: we ignore memory increase costs
		simpleRunGas(m_copyRoutine) + GasCosts::copyGas,
		// Data gas for copy routines: Some bytes are zero, but we ignore them.
		bytesRequired(m_copyRoutine) * (m_params.isCreation ? GasCosts::txDataNonZeroGas : GasCosts::createDataGas),
		// Data gas for data itself
		dataGas(toBigEndian(m_value))
	);
}

void CodeCopyMethod::execute(Assembly& _assembly, AssemblyItems& _items)
{
	bytes data = toBigEndian(m_value);
	m_copyRoutine[4] = _assembly.newData(data);
	replaceConstants(_items, m_copyRoutine);
}

AssemblyItems ComputeMethod::findRepresentation(u256 const& _value)
{
	if (_value < 0x10000)
		// Very small value, not worth computing
		return AssemblyItems{_value};
	else if (dev::bytesRequired(~_value) < dev::bytesRequired(_value))
		// Negated is shorter to represent
		return findRepresentation(~_value) + AssemblyItems{Instruction::NOT};
	else
	{
		// Decompose value into a * 2**k + b where abs(b) << 2**k
		// Is not always better, try literal and decomposition method.
		AssemblyItems routine{u256(_value)};
		bigint bestGas = gasNeeded(routine);
		for (unsigned bits = 255; bits > 8; --bits)
		{
			unsigned gapDetector = unsigned(_value >> (bits - 8)) & 0x1ff;
			if (gapDetector != 0xff && gapDetector != 0x100)
				continue;

			u256 powerOfTwo = u256(1) << bits;
			u256 upperPart = _value >> bits;
			bigint lowerPart = _value & (powerOfTwo - 1);
			if (abs(powerOfTwo - lowerPart) < lowerPart)
				lowerPart = lowerPart - powerOfTwo; // make it negative
			if (abs(lowerPart) >= (powerOfTwo >> 8))
				continue;

			AssemblyItems newRoutine;
			if (lowerPart != 0)
				newRoutine += findRepresentation(u256(abs(lowerPart)));
			newRoutine += AssemblyItems{u256(bits), u256(2), Instruction::EXP};
			if (upperPart != 1 && upperPart != 0)
				newRoutine += findRepresentation(upperPart) + AssemblyItems{Instruction::MUL};
			if (lowerPart > 0)
				newRoutine += AssemblyItems{Instruction::ADD};
			else if (lowerPart < 0)
				newRoutine.push_back(Instruction::SUB);

			bigint newGas = gasNeeded(newRoutine);
			if (newGas < bestGas)
			{
				bestGas = move(newGas);
				routine = move(newRoutine);
			}
		}
		return routine;
	}
}

bigint ComputeMethod::gasNeeded(AssemblyItems const& _routine)
{
	size_t numExps = count(_routine.begin(), _routine.end(), Instruction::EXP);
	return combineGas(
		simpleRunGas(_routine) + numExps * (GasCosts::expGas + GasCosts::expByteGas),
		// Data gas for routine: Some bytes are zero, but we ignore them.
		bytesRequired(_routine) * (m_params.isCreation ? GasCosts::txDataNonZeroGas : GasCosts::createDataGas),
		0
	);
}
