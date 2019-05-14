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
#include <libyul/AsmParser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/backends/evm/AsmCodeGen.h>
#include <liblangutil/ErrorReporter.h>

using namespace std;
using namespace dev;
using namespace dev::eth;

unsigned ConstantOptimisationMethod::optimiseConstants(
	bool _isCreation,
	size_t _runs,
	langutil::EVMVersion _evmVersion,
	Assembly& _assembly
)
{
	// TODO: design the optimiser in a way this is not needed
	AssemblyItems& _items = _assembly.items();

	unsigned optimisations = 0;
	map<AssemblyItem, size_t> pushes;
	for (AssemblyItem const& item: _items)
		if (item.type() == Push)
			pushes[item]++;
	map<u256, AssemblyItems> pendingReplacements;
	for (auto it: pushes)
	{
		u256 value = it.first.data();
		Params params;
		params.multiplicity = it.second;
		params.isCreation = _isCreation;
		params.runs = _runs;
		params.evmVersion = _evmVersion;
		AssemblyItems replacement = optimiseSingleConstant(value, params, _assembly);
		if (replacement.empty())
			continue;
		pendingReplacements[value] = move(replacement);
		optimisations++;
	}
	if (!pendingReplacements.empty())
		replaceConstants(_items, pendingReplacements);
	return optimisations;
}

AssemblyItems ConstantOptimisationMethod::optimiseSingleConstant(
	u256 const& _value,
	Params const& _params,
	Assembly& _assembly
)
{
	if (_value < 0x100)
		return {};

	LiteralMethod lit(_params, _value);
	bigint literalGas = lit.gasNeeded();
	CodeCopyMethod copy(_params, _value);
	bigint copyGas = copy.gasNeeded();
	ComputeMethod compute(_params, _value);
	bigint computeGas = compute.gasNeeded();
	if (copyGas < literalGas && copyGas < computeGas)
		return copy.execute(_assembly);
	else if (computeGas < literalGas && computeGas <= copyGas)
		return compute.execute(_assembly);
	return {};
}

bigint ConstantOptimisationMethod::simpleRunGas(AssemblyItems const& _items)
{
	bigint gas = 0;
	for (AssemblyItem const& item: _items)
		if (item.type() == Push)
			gas += GasMeter::runGas(Instruction::PUSH1);
		else if (item.type() == Operation)
		{
			if (item.instruction() == Instruction::EXP)
				gas += GasCosts::expGas;
			else
				gas += GasMeter::runGas(item.instruction());
		}
	return gas;
}

bigint ConstantOptimisationMethod::dataGas(bytes const& _data) const
{
	assertThrow(_data.size() > 0, OptimizerException, "Empty bytecode generated.");
	return bigint(GasMeter::dataGas(_data, m_params.isCreation));
}

size_t ConstantOptimisationMethod::bytesRequired(AssemblyItems const& _items)
{
	return eth::bytesRequired(_items, 3); // assume 3 byte addresses
}

void ConstantOptimisationMethod::replaceConstants(
	AssemblyItems& _items,
	map<u256, AssemblyItems> const& _replacements
)
{
	AssemblyItems replaced;
	for (AssemblyItem const& item: _items)
	{
		if (item.type() == Push)
		{
			auto it = _replacements.find(item.data());
			if (it != _replacements.end())
			{
				replaced += it->second;
				continue;
			}
		}
		replaced.push_back(item);
	}
	_items = std::move(replaced);
}

bigint LiteralMethod::gasNeeded() const
{
	return combineGas(
		simpleRunGas({Instruction::PUSH1}),
		// PUSHX plus data
		(m_params.isCreation ? GasCosts::txDataNonZeroGas : GasCosts::createDataGas) + dataGas(toCompactBigEndian(m_value, 1)),
		0
	);
}

bigint CodeCopyMethod::gasNeeded() const
{
	return combineGas(
		// Run gas: we ignore memory increase costs
		simpleRunGas(copyRoutine()) + GasCosts::copyGas,
		// Data gas for copy routines: Some bytes are zero, but we ignore them.
		bytesRequired(copyRoutine()) * (m_params.isCreation ? GasCosts::txDataNonZeroGas : GasCosts::createDataGas),
		// Data gas for data itself
		dataGas(toBigEndian(m_value))
	);
}

AssemblyItems CodeCopyMethod::execute(Assembly& _assembly) const
{
	bytes data = toBigEndian(m_value);
	AssemblyItems actualCopyRoutine = copyRoutine();
	actualCopyRoutine[4] = _assembly.newData(data);
	return actualCopyRoutine;
}

AssemblyItems const& CodeCopyMethod::copyRoutine()
{
	AssemblyItems static copyRoutine{
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
	return copyRoutine;
}

AssemblyItems ComputeMethod::yulRoutineToAssemblyItems(string const& _routine) const
{
	if (_routine.empty())
		return {};

	string routine = "{ pop(" + _routine + ") }";
	langutil::ErrorList errors;
	langutil::ErrorReporter errorReporter(errors);
	auto scanner = make_shared<langutil::Scanner>(langutil::CharStream(routine, "--CODEGEN--"));
	auto parserResult = yul::Parser(errorReporter, yul::EVMDialect::strictAssemblyForEVM(m_params.evmVersion)).parse(scanner, false);
	solAssert(parserResult, "");
	yul::AsmAnalysisInfo analysisInfo;
	bool analyzerResult = false;
	analyzerResult = yul::AsmAnalyzer(
		analysisInfo,
		errorReporter,
		boost::none,
		yul::EVMDialect::strictAssemblyForEVM(m_params.evmVersion)
	).analyze(*parserResult);
	solAssert(analyzerResult, "");
	solAssert(errorReporter.errors().empty(), "");

	Assembly _asm;
	yul::CodeGenerator::assemble(
		*parserResult,
		analysisInfo,
		_asm,
		m_params.evmVersion
	);
	solAssert(!_asm.items().empty(), "");
	solAssert(_asm.items().back() == eth::Instruction::POP, "");
	return {_asm.items().begin(), --_asm.items().end()};
}

string ComputeMethod::findRepresentation(u256 const& _value)
{
	if (_value < 0x10000)
		// Very small value, not worth computing
		return _value.str();
	else if (dev::bytesRequired(~_value) < dev::bytesRequired(_value))
		// Negated is shorter to represent
		return "not(" + findRepresentation(~_value) + ")";
	else
	{
		// Decompose value into a * 2**k + b where abs(b) << 2**k
		// Is not always better, try literal and decomposition method.
		string routine{_value.str()};
		bigint bestGas = gasNeeded(routine);
		for (unsigned bits = 255; bits > 8 && m_maxSteps > 0; --bits)
		{
			unsigned gapDetector = unsigned((_value >> (bits - 8)) & 0x1ff);
			if (gapDetector != 0xff && gapDetector != 0x100)
				continue;

			u256 powerOfTwo = u256(1) << bits;
			u256 upperPart = _value >> bits;
			bigint lowerPart = _value & (powerOfTwo - 1);
			if ((powerOfTwo - lowerPart) < lowerPart)
			{
				lowerPart = lowerPart - powerOfTwo; // make it negative
				upperPart++;
			}
			if (upperPart == 0)
				continue;
			if (abs(lowerPart) >= (powerOfTwo >> 8))
				continue;

			string newRoutine;
			if (m_params.evmVersion.hasBitwiseShifting())
				newRoutine = "shl(" + to_string(bits) + ", " + findRepresentation(upperPart) + ")";
			else
			{
				newRoutine = "exp(2, " + to_string(bits) + ")";
				if (upperPart != 1)
					newRoutine = "mul(" + findRepresentation(upperPart) + ", " + newRoutine + ")";
			}
			if (lowerPart > 0)
				newRoutine = "add(" + newRoutine + ", " + findRepresentation(u256(abs(lowerPart))) + ")";
			else if (lowerPart < 0)
				newRoutine = "sub(" + newRoutine + ", " + findRepresentation(u256(abs(lowerPart))) + ")";

			if (m_maxSteps > 0)
				m_maxSteps--;
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

bool ComputeMethod::checkRepresentation(u256 const& _value, AssemblyItems const& _routine) const
{
	// This is a tiny EVM that can only evaluate some instructions.
	vector<u256> stack;
	for (AssemblyItem const& item: _routine)
	{
		switch (item.type())
		{
		case Operation:
		{
			if (stack.size() < size_t(item.arguments()))
				return false;
			u256* sp = &stack.back();
			switch (item.instruction())
			{
			case Instruction::MUL:
				sp[-1] = sp[0] * sp[-1];
				break;
			case Instruction::EXP:
				if (sp[-1] > 0xff)
					return false;
				sp[-1] = boost::multiprecision::pow(sp[0], unsigned(sp[-1]));
				break;
			case Instruction::ADD:
				sp[-1] = sp[0] + sp[-1];
				break;
			case Instruction::SUB:
				sp[-1] = sp[0] - sp[-1];
				break;
			case Instruction::NOT:
				sp[0] = ~sp[0];
				break;
			case Instruction::SHL:
				assertThrow(
					m_params.evmVersion.hasBitwiseShifting(),
					OptimizerException,
					"Shift generated for invalid EVM version."
				);
				assertThrow(sp[0] <= u256(255), OptimizerException, "Invalid shift generated.");
				sp[-1] = u256(bigint(sp[-1]) << unsigned(sp[0]));
				break;
			case Instruction::SHR:
				assertThrow(
					m_params.evmVersion.hasBitwiseShifting(),
					OptimizerException,
					"Shift generated for invalid EVM version."
				);
				assertThrow(sp[0] <= u256(255), OptimizerException, "Invalid shift generated.");
				sp[-1] = sp[-1] >> unsigned(sp[0]);
				break;
			default:
				return false;
			}
			stack.resize(stack.size() + item.deposit());
			break;
		}
		case Push:
			stack.push_back(item.data());
			break;
		default:
			return false;
		}
	}
	return stack.size() == 1 && stack.front() == _value;
}

bigint ComputeMethod::gasNeeded(string const& _routine) const
{
	return gasNeeded(yulRoutineToAssemblyItems(_routine));
}

bigint ComputeMethod::gasNeeded(AssemblyItems const& _routine) const
{
	size_t numExps = count(_routine.begin(), _routine.end(), Instruction::EXP);
	return combineGas(
		simpleRunGas(_routine) + numExps * (GasCosts::expGas + GasCosts::expByteGas(m_params.evmVersion)),
		// Data gas for routine: Some bytes are zero, but we ignore them.
		bytesRequired(_routine) * (m_params.isCreation ? GasCosts::txDataNonZeroGas : GasCosts::createDataGas),
		0
	);
}
