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
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <libdevcore/Log.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/NameAndTypeResolver.h>
#include <libsolidity/Compiler.h>
#include <libsolidity/AST.h>

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
	ASTPointer<SourceUnit> sourceUnit;
	BOOST_REQUIRE_NO_THROW(sourceUnit = parser.parse(make_shared<Scanner>(CharStream(_sourceCode))));
	NameAndTypeResolver resolver({});
	resolver.registerDeclarations(*sourceUnit);
	for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			BOOST_REQUIRE_NO_THROW(resolver.resolveNamesAndTypes(*contract));
		}
	for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			BOOST_REQUIRE_NO_THROW(resolver.checkTypeRequirements(*contract));
		}
	for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			Compiler compiler;
			compiler.compileContract(*contract, map<ContractDefinition const*, bytes const*>{});

			// debug
			//compiler.streamAssembly(cout);
			return compiler.getAssembledBytecode();
		}
	BOOST_FAIL("No contract found in source.");
	return bytes();
}

/// Checks that @a _compiledCode is present starting from offset @a _offset in @a _expectation.
/// This is necessary since the compiler will add boilerplate add the beginning that is not
/// tested here.
void checkCodePresentAt(bytes const& _compiledCode, bytes const& _expectation, unsigned _offset)
{
	BOOST_REQUIRE(_compiledCode.size() >= _offset + _expectation.size());
	auto checkStart = _compiledCode.begin() + _offset;
	BOOST_CHECK_EQUAL_COLLECTIONS(checkStart, checkStart + _expectation.size(),
								  _expectation.begin(), _expectation.end());
}

} // end anonymous namespace

BOOST_AUTO_TEST_SUITE(SolidityCompiler)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* sourceCode = "contract test {\n"
							 "  function f() { var x = 2; }\n"
							 "}\n";
	bytes code = compileContract(sourceCode);

	unsigned boilerplateSize = 69;
	bytes expectation({byte(Instruction::JUMPDEST),
					   byte(Instruction::PUSH1), 0x0, // initialize local variable x
					   byte(Instruction::PUSH1), 0x2,
					   byte(Instruction::SWAP1),
					   byte(Instruction::POP),
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::POP),
					   byte(Instruction::JUMP)});
	checkCodePresentAt(code, expectation, boilerplateSize);
}

BOOST_AUTO_TEST_CASE(ifStatement)
{
	char const* sourceCode = "contract test {\n"
							 "  function f() { bool x; if (x) 77; else if (!x) 78; else 79; }"
							 "}\n";
	bytes code = compileContract(sourceCode);
	unsigned shift = 56;
	unsigned boilerplateSize = 69;
	bytes expectation({byte(Instruction::JUMPDEST),
					   byte(Instruction::PUSH1), 0x0,
					   byte(Instruction::DUP1),
					   byte(Instruction::PUSH1), byte(0x1b + shift), // "true" target
					   byte(Instruction::JUMPI),
					   // new check "else if" condition
					   byte(Instruction::DUP1),
					   byte(Instruction::ISZERO),
					   byte(Instruction::PUSH1), byte(0x13 + shift),
					   byte(Instruction::JUMPI),
					   // "else" body
					   byte(Instruction::PUSH1), 0x4f,
					   byte(Instruction::POP),
					   byte(Instruction::PUSH1), byte(0x17 + shift), // exit path of second part
					   byte(Instruction::JUMP),
					   // "else if" body
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::PUSH1), 0x4e,
					   byte(Instruction::POP),
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::PUSH1), byte(0x1f + shift),
					   byte(Instruction::JUMP),
					   // "if" body
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::PUSH1), 0x4d,
					   byte(Instruction::POP),
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::POP),
					   byte(Instruction::JUMP)});
	checkCodePresentAt(code, expectation, boilerplateSize);
}

BOOST_AUTO_TEST_CASE(loops)
{
	char const* sourceCode = "contract test {\n"
							 "  function f() { while(true){1;break;2;continue;3;return;4;} }"
							 "}\n";
	bytes code = compileContract(sourceCode);
	unsigned shift = 56;
	unsigned boilerplateSize = 69;
	bytes expectation({byte(Instruction::JUMPDEST),
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::PUSH1), 0x1,
					   byte(Instruction::ISZERO),
					   byte(Instruction::PUSH1), byte(0x21 + shift),
					   byte(Instruction::JUMPI),
					   byte(Instruction::PUSH1), 0x1,
					   byte(Instruction::POP),
					   byte(Instruction::PUSH1), byte(0x21 + shift),
					   byte(Instruction::JUMP), // break
					   byte(Instruction::PUSH1), 0x2,
					   byte(Instruction::POP),
					   byte(Instruction::PUSH1), byte(0x2 + shift),
					   byte(Instruction::JUMP), // continue
					   byte(Instruction::PUSH1), 0x3,
					   byte(Instruction::POP),
					   byte(Instruction::PUSH1), byte(0x22 + shift),
					   byte(Instruction::JUMP), // return
					   byte(Instruction::PUSH1), 0x4,
					   byte(Instruction::POP),
					   byte(Instruction::PUSH1), byte(0x2 + shift),
					   byte(Instruction::JUMP),
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::JUMPDEST),
					   byte(Instruction::JUMP)});

	checkCodePresentAt(code, expectation, boilerplateSize);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces

