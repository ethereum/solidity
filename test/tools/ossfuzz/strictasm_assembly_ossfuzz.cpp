// SPDX-License-Identifier: GPL-3.0

#include <libyul/AssemblyStack.h>
#include <liblangutil/EVMVersion.h>
#include <libyul/backends/evm/EVMCodeTransform.h>

using namespace solidity;
using namespace solidity::yul;
using namespace std;

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	if (_size > 600)
		return 0;

	YulStringRepository::reset();

	string input(reinterpret_cast<char const*>(_data), _size);
	AssemblyStack stack(
		langutil::EVMVersion(),
		AssemblyStack::Language::StrictAssembly,
		solidity::frontend::OptimiserSettings::full()
	);

	if (!stack.parseAndAnalyze("source", input))
		return 0;

	try
	{
		MachineAssemblyObject obj = stack.assemble(AssemblyStack::Machine::EVM);
		solAssert(obj.bytecode, "");
	}
	catch (StackTooDeepError const&)
	{

	}

	return 0;
}
