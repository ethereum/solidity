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

#include <libyul/YulStack.h>
#include <libyul/backends/evm/EVMCodeTransform.h>

#include <liblangutil/DebugInfoSelection.h>
#include <liblangutil/EVMVersion.h>

using namespace solidity;
using namespace solidity::yul;

// Prototype as we can't use the FuzzerInterface.h header.
extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size);

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	if (_size > 600)
		return 0;

	YulStringRepository::reset();

	std::string input(reinterpret_cast<char const*>(_data), _size);
	YulStack stack(
		langutil::EVMVersion(),
		std::nullopt,
		YulStack::Language::StrictAssembly,
		solidity::frontend::OptimiserSettings::minimal(),
		langutil::DebugInfoSelection::ExceptExperimental()
	);

	if (!stack.parseAndAnalyze("source", input))
		return 0;

	try
	{
		MachineAssemblyObject obj = stack.assemble(YulStack::Machine::EVM);
		solAssert(obj.bytecode, "");
	}
	catch (StackTooDeepError const&)
	{

	}

	return 0;
}
