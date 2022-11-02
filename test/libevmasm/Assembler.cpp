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
 * @author Alex Beregszaszi
 * @date 2018
 * Tests for the assembler.
 */
#include <test/Common.h>

#include <libevmasm/Assembly.h>
#include <libsolutil/JSON.h>
#include <libevmasm/Disassemble.h>
#include <libyul/Exceptions.h>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <memory>
#include <string>
#include <tuple>

using namespace std;
using namespace solidity::langutil;
using namespace solidity::evmasm;

namespace solidity::frontend::test
{

namespace
{
	void checkCompilation(evmasm::Assembly const& _assembly)
	{
		LinkerObject output = _assembly.assemble();
		BOOST_CHECK(output.bytecode.size() > 0);
		BOOST_CHECK(output.toHex().length() > 0);
	}
}

BOOST_AUTO_TEST_SUITE(Assembler)

BOOST_AUTO_TEST_CASE(all_assembly_items)
{
	EVMVersion evmVersion = solidity::test::CommonOptions::get().evmVersion();
	Assembly _assembly{evmVersion, false, {}};
	_assembly.setSourceList({"root.asm", "sub.asm", "verbatim.asm"});
	auto root_asm = make_shared<string>("root.asm");
	_assembly.setSourceLocation({1, 3, root_asm});

	Assembly _subAsm{evmVersion, false, {}};
	auto sub_asm = make_shared<string>("sub.asm");
	_subAsm.setSourceLocation({6, 8, sub_asm});

	Assembly _verbatimAsm(evmVersion, true, "");
	auto verbatim_asm = make_shared<string>("verbatim.asm");
	_verbatimAsm.setSourceLocation({8, 18, verbatim_asm});

	// PushImmutable
	_subAsm.appendImmutable("someImmutable");
	_subAsm.append(AssemblyItem(PushTag, 0));
	_subAsm.append(Instruction::INVALID);
	shared_ptr<Assembly> _subAsmPtr = make_shared<Assembly>(_subAsm);

	_verbatimAsm.appendVerbatim({0xff,0xff}, 0, 0);
	_verbatimAsm.appendVerbatim({0x74, 0x65, 0x73, 0x74}, 0, 1);
	_verbatimAsm.append(Instruction::MSTORE);
	shared_ptr<Assembly> _verbatimAsmPtr = make_shared<Assembly>(_verbatimAsm);

	// Tag
	auto tag = _assembly.newTag();
	_assembly.append(tag);
	// Operation
	_assembly.append(u256(1));
	_assembly.append(u256(2));
	// Push
	auto keccak256 = AssemblyItem(Instruction::KECCAK256);
	_assembly.m_currentModifierDepth = 1;
	_assembly.append(keccak256);
	_assembly.m_currentModifierDepth = 0;
	// PushProgramSize
	_assembly.appendProgramSize();
	// PushLibraryAddress
	_assembly.appendLibraryAddress("someLibrary");
	// PushTag + Operation
	_assembly.appendJump(tag);
	// PushData
	_assembly.append(bytes{0x1, 0x2, 0x3, 0x4});
	// PushSubSize
	auto sub = _assembly.appendSubroutine(_subAsmPtr);
	// PushSub
	_assembly.pushSubroutineOffset(static_cast<size_t>(sub.data()));
	// PushSubSize
	auto verbatim_sub = _assembly.appendSubroutine(_verbatimAsmPtr);
	// PushSub
	_assembly.pushSubroutineOffset(static_cast<size_t>(verbatim_sub.data()));
	// PushDeployTimeAddress
	_assembly.append(PushDeployTimeAddress);
	// AssignImmutable.
	// Note that since there is no reference to "someOtherImmutable", this will just compile to two POPs in the hex output.
	_assembly.appendImmutableAssignment("someOtherImmutable");
	_assembly.append(u256(2));
	_assembly.appendImmutableAssignment("someImmutable");
	// Operation
	_assembly.append(Instruction::STOP);
	_assembly.appendToAuxiliaryData(bytes{0x42, 0x66});
	_assembly.appendToAuxiliaryData(bytes{0xee, 0xaa});

	_assembly.m_currentModifierDepth = 2;
	_assembly.appendJump(tag);
	_assembly.m_currentModifierDepth = 0;

	checkCompilation(_assembly);

	BOOST_CHECK_EQUAL(
		_assembly.assemble().toHex(),
		"5b6001600220607f73__$bf005014d9d0f534b8fcb268bd84c491a2$__"
		"60005660776024604c600760707300000000000000000000000000000000000000005050"
		"600260010152"
		"006000"
		"56fe"
		"7f0000000000000000000000000000000000000000000000000000000000000000"
		"6000feffff7465737452010203044266eeaa"
	);
	BOOST_CHECK_EQUAL(
		_assembly.assemblyString(),
		"    /* \"root.asm\":1:3   */\n"
		"tag_1:\n"
		"  keccak256(0x02, 0x01)\n"
		"  bytecodeSize\n"
		"  linkerSymbol(\"bf005014d9d0f534b8fcb268bd84c491a2380f4acd260d1ccfe9cd8201f7e994\")\n"
		"  jump(tag_1)\n"
		"  data_a6885b3731702da62e8e4a8f584ac46a7f6822f4e2ba50fba902f67b1588d23b\n"
		"  dataSize(sub_0)\n"
		"  dataOffset(sub_0)\n"
		"  dataSize(sub_1)\n"
		"  dataOffset(sub_1)\n"
		"  deployTimeAddress()\n"
		"  assignImmutable(\"0xc3978657661c4d8e32e3d5f42597c009f0d3859e9f9d0d94325268f9799e2bfb\")\n"
		"  0x02\n"
		"  assignImmutable(\"0x26f2c0195e9d408feff3abd77d83f2971f3c9a18d1e8a9437c7835ae4211fc9f\")\n"
		"  stop\n"
		"  jump(tag_1)\n"
		"stop\n"
		"data_a6885b3731702da62e8e4a8f584ac46a7f6822f4e2ba50fba902f67b1588d23b 01020304\n"
		"\n"
		"sub_0: assembly {\n"
		"        /* \"sub.asm\":6:8   */\n"
		"      immutable(\"0x26f2c0195e9d408feff3abd77d83f2971f3c9a18d1e8a9437c7835ae4211fc9f\")\n"
		"      tag_0\n"
		"      invalid\n"
		"}\n"
		"\n"
		"sub_1: assembly {\n"
		"        /* \"verbatim.asm\":8:18   */\n"
		"      verbatimbytecode_ffff\n"
		"      verbatimbytecode_74657374\n"
		"      mstore\n"
		"}\n"
		"\n"
		"auxdata: 0x4266eeaa\n"
	);
	string json{
		"{\".auxdata\":\"4266eeaa\",\".code\":["
		"{\"begin\":1,\"end\":3,\"name\":\"tag\",\"source\":0,\"value\":\"1\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"JUMPDEST\",\"source\":0},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH\",\"source\":0,\"value\":\"1\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH\",\"source\":0,\"value\":\"2\"},"
		"{\"begin\":1,\"end\":3,\"modifierDepth\":1,\"name\":\"KECCAK256\",\"source\":0},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSHSIZE\",\"source\":0},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSHLIB\",\"source\":0,\"value\":\"someLibrary\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH [tag]\",\"source\":0,\"value\":\"1\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"JUMP\",\"source\":0},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH data\",\"source\":0,\"value\":\"A6885B3731702DA62E8E4A8F584AC46A7F6822F4E2BA50FBA902F67B1588D23B\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH #[$]\",\"source\":0,\"value\":\"0000000000000000000000000000000000000000000000000000000000000000\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH [$]\",\"source\":0,\"value\":\"0000000000000000000000000000000000000000000000000000000000000000\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH #[$]\",\"source\":0,\"value\":\"0000000000000000000000000000000000000000000000000000000000000001\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH [$]\",\"source\":0,\"value\":\"0000000000000000000000000000000000000000000000000000000000000001\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSHDEPLOYADDRESS\",\"source\":0},"
		"{\"begin\":1,\"end\":3,\"name\":\"ASSIGNIMMUTABLE\",\"source\":0,\"value\":\"someOtherImmutable\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH\",\"source\":0,\"value\":\"2\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"ASSIGNIMMUTABLE\",\"source\":0,\"value\":\"someImmutable\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"STOP\",\"source\":0},"
		"{\"begin\":1,\"end\":3,\"modifierDepth\":2,\"name\":\"PUSH [tag]\",\"source\":0,\"value\":\"1\"},{\"begin\":1,\"end\":3,\"modifierDepth\":2,\"name\":\"JUMP\",\"source\":0}"
		"],\".data\":{\"0\":{\".code\":["
		"{\"begin\":6,\"end\":8,\"name\":\"PUSHIMMUTABLE\",\"source\":1,\"value\":\"someImmutable\"},"
		"{\"begin\":6,\"end\":8,\"name\":\"PUSH [ErrorTag]\",\"source\":1},"
		"{\"begin\":6,\"end\":8,\"name\":\"INVALID\",\"source\":1}"
		"]},"
		"\"1\":{\".code\":["
		"{\"begin\":8,\"end\":18,\"name\":\"VERBATIM\",\"source\":2,\"value\":\"ffff\"},"
		"{\"begin\":8,\"end\":18,\"name\":\"VERBATIM\",\"source\":2,\"value\":\"74657374\"},"
		"{\"begin\":8,\"end\":18,\"name\":\"MSTORE\",\"source\":2}"
		"]},\"A6885B3731702DA62E8E4A8F584AC46A7F6822F4E2BA50FBA902F67B1588D23B\":\"01020304\"},\"sourceList\":[\"root.asm\",\"sub.asm\",\"verbatim.asm\"]}"
	};
	Json::Value jsonValue;
	BOOST_CHECK(util::jsonParseStrict(json, jsonValue));
	BOOST_CHECK_EQUAL(util::jsonCompactPrint(_assembly.assemblyJSON()), util::jsonCompactPrint(jsonValue));
}

BOOST_AUTO_TEST_CASE(immutables_and_its_source_maps)
{
	EVMVersion evmVersion = solidity::test::CommonOptions::get().evmVersion();
	// Tests for 1, 2, 3 number of immutables.
	for (int numImmutables = 1; numImmutables <= 3; ++numImmutables)
	{
		BOOST_TEST_MESSAGE("NumImmutables: "s + to_string(numImmutables));
		// Tests for the cases 1, 2, 3 occurrences of an immutable reference.
		for (int numActualRefs = 1; numActualRefs <= 3; ++numActualRefs)
		{
			BOOST_TEST_MESSAGE("NumActualRefs: "s + to_string(numActualRefs));
			auto const NumExpectedMappings =
				(
					2 +                        // PUSH <a> PUSH <b>
					(numActualRefs - 1) * 5 +  // DUP DUP PUSH <n> ADD MTOSRE
					3                          // PUSH <n> ADD MSTORkhbE
				) * numImmutables;

			auto constexpr NumSubs = 1;
			auto constexpr NumOpcodesWithoutMappings =
				NumSubs +                  // PUSH <addr> for every sub assembly
				1;                         // INVALID

			auto assemblyName = make_shared<string>("root.asm");
			auto subName = make_shared<string>("sub.asm");

			map<string, unsigned> indices = {
				{ *assemblyName, 0 },
				{ *subName, 1 }
			};

			auto subAsm = make_shared<Assembly>(evmVersion, false, string{});
			for (char i = 0; i < numImmutables; ++i)
			{
				for (int r = 0; r < numActualRefs; ++r)
				{
					subAsm->setSourceLocation(SourceLocation{10*i, 10*i + 6 + r, subName});
					subAsm->appendImmutable(string(1, char('a' + i))); // "a", "b", ...
				}
			}

			Assembly assembly{evmVersion, true, {}};
			for (char i = 1; i <= numImmutables; ++i)
			{
				assembly.setSourceLocation({10*i, 10*i + 3+i, assemblyName});
				assembly.append(u256(0x71));              // immutble value
				assembly.append(u256(0));                 // target... modules?
				assembly.appendImmutableAssignment(string(1, char('a' + i - 1)));
			}

			assembly.appendSubroutine(subAsm);

			checkCompilation(assembly);

			string const sourceMappings = AssemblyItem::computeSourceMapping(assembly.items(), indices);
			auto const numberOfMappings = std::count(sourceMappings.begin(), sourceMappings.end(), ';');

			LinkerObject const& obj = assembly.assemble();
			string const disassembly = disassemble(obj.bytecode, evmVersion, "\n");
			auto const numberOfOpcodes = std::count(disassembly.begin(), disassembly.end(), '\n');

			#if 0 // {{{ debug prints
			{
				LinkerObject const& subObj = assembly.sub(0).assemble();
				string const subDisassembly = disassemble(subObj.bytecode, "\n");
				cout << '\n';
				cout << "### immutables: " << numImmutables << ", refs: " << numActualRefs << '\n';
				cout << " - srcmap: \"" << sourceMappings << "\"\n";
				cout << " - src mappings: " << numberOfMappings << '\n';
				cout << " - opcodes: " << numberOfOpcodes << '\n';
				cout << " - subs: " << assembly.numSubs() << '\n';
				cout << " - sub opcodes " << std::count(subDisassembly.begin(), subDisassembly.end(), '\n') << '\n';
				cout << " - sub srcmaps " << AssemblyItem::computeSourceMapping(subAsm->items(), indices) << '\n';
				cout << " - main bytecode:\n\t" << disassemble(obj.bytecode, "\n\t");
				cout << "\r - sub bytecode:\n\t" << disassemble(subObj.bytecode, "\n\t");
			}
			#endif // }}}

			BOOST_REQUIRE_EQUAL(NumExpectedMappings, numberOfMappings);
			BOOST_REQUIRE_EQUAL(NumExpectedMappings, numberOfOpcodes - NumOpcodesWithoutMappings);
		}
	}
}

BOOST_AUTO_TEST_CASE(immutable)
{
	EVMVersion evmVersion = solidity::test::CommonOptions::get().evmVersion();
	Assembly _assembly{evmVersion, true, {}};
	_assembly.setSourceList({"root.asm", "sub.asm"});
	auto root_asm = make_shared<string>("root.asm");
	_assembly.setSourceLocation({1, 3, root_asm});

	Assembly _subAsm{evmVersion, false, {}};
	auto sub_asm = make_shared<string>("sub.asm");
	_subAsm.setSourceLocation({6, 8, sub_asm});
	_subAsm.appendImmutable("someImmutable");
	_subAsm.appendImmutable("someOtherImmutable");
	_subAsm.appendImmutable("someImmutable");
	shared_ptr<Assembly> _subAsmPtr = make_shared<Assembly>(_subAsm);

	_assembly.append(u256(42));
	_assembly.append(u256(0));
	_assembly.appendImmutableAssignment("someImmutable");
	_assembly.append(u256(23));
	_assembly.append(u256(0));
	_assembly.appendImmutableAssignment("someOtherImmutable");

	auto sub = _assembly.appendSubroutine(_subAsmPtr);
	_assembly.pushSubroutineOffset(static_cast<size_t>(sub.data()));

	checkCompilation(_assembly);

	string genericPush0 = evmVersion.hasPush0() ? "5f" : "6000";
	// PUSH1 0x1b v/s PUSH1 0x19
	string dataOffset = evmVersion.hasPush0() ? "6019" : "601b" ;

	BOOST_CHECK_EQUAL(
		_assembly.assemble().toHex(),
		// root.asm
		// assign "someImmutable"
		"602a" + // PUSH1 42 - value for someImmutable
		genericPush0 + // PUSH1 0 - offset of code into which to insert the immutable
		"8181" // DUP2 DUP2
		"6001" // PUSH1 1 - offset of first someImmutable in sub_0
		"01" // ADD - add offset of immutable to offset of code
		"52" // MSTORE
		"6043" // PUSH1 67 - offset of second someImmutable in sub_0
		"01" // ADD - add offset of immutable to offset of code
		"52" // MSTORE
		// assign "someOtherImmutable"
		"6017" + // PUSH1 23 - value for someOtherImmutable
		genericPush0 + // PUSH1 0 - offset of code into which to insert the immutable
		"6022" // PUSH1 34 - offset of someOtherImmutable in sub_0
		"01" // ADD - add offset of immutable to offset of code
		"52" // MSTORE
		"6063" + // PUSH1 0x63 - dataSize(sub_0)
		dataOffset +  // PUSH1 0x23 - dataOffset(sub_0)
		"fe" // INVALID
		// end of root.asm
		// sub.asm
		"7f0000000000000000000000000000000000000000000000000000000000000000" // PUSHIMMUTABLE someImmutable - data at offset 1
		"7f0000000000000000000000000000000000000000000000000000000000000000" // PUSHIMMUTABLE someOtherImmutable - data at offset 34
		"7f0000000000000000000000000000000000000000000000000000000000000000" // PUSHIMMUTABLE someImmutable - data at offset 67
	);
	BOOST_CHECK_EQUAL(
		_assembly.assemblyString(),
		"    /* \"root.asm\":1:3   */\n"
		"  0x2a\n"
		"  0x00\n"
		"  assignImmutable(\"0x26f2c0195e9d408feff3abd77d83f2971f3c9a18d1e8a9437c7835ae4211fc9f\")\n"
		"  0x17\n"
		"  0x00\n"
		"  assignImmutable(\"0xc3978657661c4d8e32e3d5f42597c009f0d3859e9f9d0d94325268f9799e2bfb\")\n"
		"  dataSize(sub_0)\n"
		"  dataOffset(sub_0)\n"
		"stop\n"
		"\n"
		"sub_0: assembly {\n"
		"        /* \"sub.asm\":6:8   */\n"
		"      immutable(\"0x26f2c0195e9d408feff3abd77d83f2971f3c9a18d1e8a9437c7835ae4211fc9f\")\n"
		"      immutable(\"0xc3978657661c4d8e32e3d5f42597c009f0d3859e9f9d0d94325268f9799e2bfb\")\n"
		"      immutable(\"0x26f2c0195e9d408feff3abd77d83f2971f3c9a18d1e8a9437c7835ae4211fc9f\")\n"
		"}\n"
	);
	BOOST_CHECK_EQUAL(
		util::jsonCompactPrint(_assembly.assemblyJSON()),
		"{\".code\":["
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH\",\"source\":0,\"value\":\"2A\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH\",\"source\":0,\"value\":\"0\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"ASSIGNIMMUTABLE\",\"source\":0,\"value\":\"someImmutable\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH\",\"source\":0,\"value\":\"17\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH\",\"source\":0,\"value\":\"0\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"ASSIGNIMMUTABLE\",\"source\":0,\"value\":\"someOtherImmutable\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH #[$]\",\"source\":0,\"value\":\"0000000000000000000000000000000000000000000000000000000000000000\"},"
		"{\"begin\":1,\"end\":3,\"name\":\"PUSH [$]\",\"source\":0,\"value\":\"0000000000000000000000000000000000000000000000000000000000000000\"}"
		"],\".data\":{\"0\":{\".code\":["
		"{\"begin\":6,\"end\":8,\"name\":\"PUSHIMMUTABLE\",\"source\":1,\"value\":\"someImmutable\"},"
		"{\"begin\":6,\"end\":8,\"name\":\"PUSHIMMUTABLE\",\"source\":1,\"value\":\"someOtherImmutable\"},"
		"{\"begin\":6,\"end\":8,\"name\":\"PUSHIMMUTABLE\",\"source\":1,\"value\":\"someImmutable\"}"
		"]}},\"sourceList\":[\"root.asm\",\"sub.asm\"]}"
	);
}

BOOST_AUTO_TEST_CASE(subobject_encode_decode)
{
	EVMVersion evmVersion = solidity::test::CommonOptions::get().evmVersion();
	Assembly assembly{evmVersion, true, {}};

	shared_ptr<Assembly> subAsmPtr = make_shared<Assembly>(evmVersion, false, string{});
	shared_ptr<Assembly> subSubAsmPtr = make_shared<Assembly>(evmVersion, false, string{});

	assembly.appendSubroutine(subAsmPtr);
	subAsmPtr->appendSubroutine(subSubAsmPtr);

	BOOST_CHECK(assembly.encodeSubPath({0}) == 0);
	BOOST_REQUIRE_THROW(assembly.encodeSubPath({1}), solidity::evmasm::AssemblyException);
	BOOST_REQUIRE_THROW(assembly.decodeSubPath(1), solidity::evmasm::AssemblyException);

	vector<size_t> subPath{0, 0};
	BOOST_CHECK(assembly.decodeSubPath(assembly.encodeSubPath(subPath)) == subPath);
}

BOOST_AUTO_TEST_SUITE_END()

} // end namespaces
