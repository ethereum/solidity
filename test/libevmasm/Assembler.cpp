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
 * @author Alex Beregszaszi
 * @date 2018
 * Tests for the assembler.
 */

#include <libdevcore/JSON.h>
#include <libevmasm/Assembly.h>

#include <boost/test/unit_test.hpp>

#include <string>
#include <tuple>
#include <memory>

using namespace std;
using namespace langutil;
using namespace dev::eth;

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{
	void checkCompilation(::dev::eth::Assembly const& _assembly)
	{
		LinkerObject output = _assembly.assemble();
		BOOST_CHECK(output.bytecode.size() > 0);
		BOOST_CHECK(output.toHex().length() > 0);
	}
}

BOOST_AUTO_TEST_SUITE(Assembler)

BOOST_AUTO_TEST_CASE(all_assembly_items)
{
	Assembly _assembly;
	auto root_asm = make_shared<CharStream>("", "root.asm");
	_assembly.setSourceLocation(SourceLocation(1, 3, root_asm));

	Assembly _subAsm;
	auto sub_asm = make_shared<CharStream>("", "sub.asm");
	_subAsm.setSourceLocation(SourceLocation(6, 8, sub_asm));
	_subAsm.append(Instruction::INVALID);
	shared_ptr<Assembly> _subAsmPtr = make_shared<Assembly>(_subAsm);

	// Tag
	auto tag = _assembly.newTag();
	_assembly.append(tag);
	// Operation
	_assembly.append(u256(1));
	_assembly.append(u256(2));
	// Push
	_assembly.append(Instruction::KECCAK256);
	// PushProgramSize
	_assembly.appendProgramSize();
	// PushLibraryAddress
	_assembly.appendLibraryAddress("someLibrary");
	// PushTag + Operation
	_assembly.appendJump(tag);
	// PushString
	_assembly.append("Unused feature for pushing string");
	// PushData
	_assembly.append(bytes{0x1, 0x2, 0x3, 0x4});
	// PushSubSize
	auto sub = _assembly.appendSubroutine(_subAsmPtr);
	// PushSub
	_assembly.pushSubroutineOffset(size_t(sub.data()));
	// PushDeployTimeAddress
	_assembly.append(PushDeployTimeAddress);
	// Operation
	_assembly.append(Instruction::STOP);
	_assembly.appendAuxiliaryDataToEnd(bytes{0x42, 0x66});
	_assembly.appendAuxiliaryDataToEnd(bytes{0xee, 0xaa});

	checkCompilation(_assembly);

	BOOST_CHECK_EQUAL(
		_assembly.assemble().toHex(),
		"5b6001600220606773__$bf005014d9d0f534b8fcb268bd84c491a2$__"
		"6000567f556e75736564206665617475726520666f722070757368696e"
		"6720737472696e605f6001605e73000000000000000000000000000000000000000000fe"
		"fe010203044266eeaa"
	);
	BOOST_CHECK_EQUAL(
		_assembly.assemblyString(),
		"    /* \"root.asm\":1:3   */\n"
		"tag_1:\n"
		"  keccak256(0x02, 0x01)\n"
		"  bytecodeSize\n"
		"  linkerSymbol(\"bf005014d9d0f534b8fcb268bd84c491a2380f4acd260d1ccfe9cd8201f7e994\")\n"
		"  jump(tag_1)\n"
		"  data_027497964124140851e8a9992ba16b5c1aaf9730b78d6036c8d65e3bb5ea4c8f\n"
		"  data_a6885b3731702da62e8e4a8f584ac46a7f6822f4e2ba50fba902f67b1588d23b\n"
		"  dataSize(sub_0)\n"
		"  dataOffset(sub_0)\n"
		"  deployTimeAddress()\n"
		"  stop\n"
		"stop\n"
		"data_a6885b3731702da62e8e4a8f584ac46a7f6822f4e2ba50fba902f67b1588d23b 01020304\n"
		"\n"
		"sub_0: assembly {\n"
		"        /* \"sub.asm\":6:8   */\n"
		"      invalid\n"
		"}\n"
		"\n"
		"auxdata: 0x4266eeaa\n"
	);
	BOOST_CHECK_EQUAL(
		dev::jsonCompactPrint(_assembly.assemblyJSON()),
		"{\".auxdata\":\"4266eeaa\",\".code\":[{\"begin\":1,\"end\":3,\"name\":\"tag\",\"value\":\"1\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"JUMPDEST\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH\",\"value\":\"1\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH\",\"value\":\"2\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"KECCAK256\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSHSIZE\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSHLIB\",\"value\":\"someLibrary\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH [tag]\",\"value\":\"1\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"JUMP\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH tag\",\"value\":\"Unused feature for pushing string\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH data\",\"value\":\"A6885B3731702DA62E8E4A8F584AC46A7F6822F4E2BA50FBA902F67B1588D23B\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH #[$]\",\"value\":\"0000000000000000000000000000000000000000000000000000000000000000\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH [$]\",\"value\":\"0000000000000000000000000000000000000000000000000000000000000000\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSHDEPLOYADDRESS\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"STOP\"}],"
		"\".data\":{\"0\":{\".code\":[{\"begin\":6,\"end\":8,\"name\":\"INVALID\"}]},"
		"\"A6885B3731702DA62E8E4A8F584AC46A7F6822F4E2BA50FBA902F67B1588D23B\":\"01020304\"}}"
	);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
