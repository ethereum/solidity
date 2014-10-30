
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
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Unit tests for the solidity compiler.
 */

#include <string>

#include <libdevcore/Log.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/NameAndTypeResolver.h>
#include <libsolidity/Compiler.h>
#include <libsolidity/AST.h>
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace dev::eth;

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{

bytes compileContract(const string& _sourceCode)
{
	Parser parser;
	ASTPointer<ContractDefinition> contract;
	BOOST_REQUIRE_NO_THROW(contract = parser.parse(make_shared<Scanner>(CharStream(_sourceCode))));
	NameAndTypeResolver resolver;
	BOOST_REQUIRE_NO_THROW(resolver.resolveNamesAndTypes(*contract));

	bytes instructions = Compiler::compile(*contract);
	// debug
	//cout << eth::disassemble(instructions) << endl;
	return instructions;
}

} // end anonymous namespace

BOOST_AUTO_TEST_SUITE(SolidityCompiler)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* sourceCode = "contract test {\n"
							 "  function f() { var x = 2; }\n"
							 "}\n";
	bytes code = compileContract(sourceCode);

	bytes expectation({byte(Instruction::JUMPDEST),
					   byte(Instruction::PUSH1), 0x0, // initialize local variable x
					   byte(Instruction::PUSH1), 0x2,
					   byte(Instruction::SWAP1),
					   byte(Instruction::POP),
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::POP),
					   byte(Instruction::JUMP)});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(different_argument_numbers)
{
	char const* sourceCode = "contract test {\n"
							 "  function f(uint a, uint b, uint c) returns(uint d) { return b; }\n"
							 "  function g() returns (uint e, uint h) { h = f(1, 2, 3); }\n"
							 "}\n";
	bytes code = compileContract(sourceCode);

	bytes expectation({byte(Instruction::JUMPDEST),
					   byte(Instruction::PUSH1), 0x0, // initialize return variable d
					   byte(Instruction::DUP3),
					   byte(Instruction::SWAP1), // assign b to d
					   byte(Instruction::POP),
					   byte(Instruction::PUSH1), 0xa, // jump to return
					   byte(Instruction::JUMP),
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::SWAP4), // store d and fetch return address
					   byte(Instruction::SWAP3), // store return address
					   byte(Instruction::POP),
					   byte(Instruction::POP),
					   byte(Instruction::POP),
					   byte(Instruction::JUMP), // end of f
					   byte(Instruction::JUMPDEST), // beginning of g
					   byte(Instruction::PUSH1), 0x0,
					   byte(Instruction::DUP1), // initialized e and h
					   byte(Instruction::PUSH1), 0x20, // ret address
					   byte(Instruction::PUSH1), 0x1,
					   byte(Instruction::PUSH1), 0x2,
					   byte(Instruction::PUSH1), 0x3,
					   byte(Instruction::PUSH1), 0x1,
					   // stack here: ret e h 0x20 1 2 3 0x1
					   byte(Instruction::JUMP),
					   byte(Instruction::JUMPDEST),
					   // stack here: ret e h f(1,2,3)
					   byte(Instruction::DUP2),
					   byte(Instruction::POP),
					   byte(Instruction::SWAP1),
					   // stack here: ret e f(1,2,3) h
					   byte(Instruction::POP),
					   byte(Instruction::DUP1), // retrieve it again as "value of expression"
					   byte(Instruction::POP), // end of assignment
					   // stack here: ret e f(1,2,3)
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::SWAP1),
					   // ret e f(1,2,3)
					   byte(Instruction::SWAP2),
					   // f(1,2,3) e ret
					   byte(Instruction::JUMP) // end of g
					   });
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(ifStatement)
{
	char const* sourceCode = "contract test {\n"
							 "  function f() { bool x; if (x) 77; else if (!x) 78; else 79; }"
							 "}\n";
	bytes code = compileContract(sourceCode);

	bytes expectation({byte(Instruction::JUMPDEST),
					   byte(Instruction::PUSH1), 0x0,
					   byte(Instruction::DUP1),
					   byte(Instruction::PUSH1), 0x1b, // "true" target
					   byte(Instruction::JUMPI),
					   // new check "else if" condition
					   byte(Instruction::DUP1),
					   byte(Instruction::NOT),
					   byte(Instruction::PUSH1), 0x13,
					   byte(Instruction::JUMPI),
					   // "else" body
					   byte(Instruction::PUSH1), 0x4f,
					   byte(Instruction::POP),
					   byte(Instruction::PUSH1), 0x17, // exit path of second part
					   byte(Instruction::JUMP),
					   // "else if" body
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::PUSH1), 0x4e,
					   byte(Instruction::POP),
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::PUSH1), 0x1f,
					   byte(Instruction::JUMP),
					   // "if" body
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::PUSH1), 0x4d,
					   byte(Instruction::POP),
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::POP),
					   byte(Instruction::JUMP)});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(loops)
{
	char const* sourceCode = "contract test {\n"
							 "  function f() { while(true){1;break;2;continue;3;return;4;} }"
							 "}\n";
	bytes code = compileContract(sourceCode);

	bytes expectation({byte(Instruction::JUMPDEST),
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::PUSH1), 0x1,
					   byte(Instruction::NOT),
					   byte(Instruction::PUSH1), 0x21,
					   byte(Instruction::JUMPI),
					   byte(Instruction::PUSH1), 0x1,
					   byte(Instruction::POP),
					   byte(Instruction::PUSH1), 0x21,
					   byte(Instruction::JUMP), // break
					   byte(Instruction::PUSH1), 0x2,
					   byte(Instruction::POP),
					   byte(Instruction::PUSH1), 0x2,
					   byte(Instruction::JUMP), // continue
					   byte(Instruction::PUSH1), 0x3,
					   byte(Instruction::POP),
					   byte(Instruction::PUSH1), 0x22,
					   byte(Instruction::JUMP), // return
					   byte(Instruction::PUSH1), 0x4,
					   byte(Instruction::POP),
					   byte(Instruction::PUSH1), 0x2,
					   byte(Instruction::JUMP),
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::JUMP)});

	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces

