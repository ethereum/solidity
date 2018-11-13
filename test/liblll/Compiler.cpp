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
 * @date 2017
 * Unit tests for the LLL compiler.
 */

#include <test/Options.h>

#include <libdevcore/FixedHash.h>

#include <liblll/Compiler.h>

#include <boost/test/unit_test.hpp>

#include <string>
#include <memory>

using namespace std;

namespace dev
{
namespace lll
{
namespace test
{

namespace
{

bool successCompile(string const& _sourceCode)
{
	vector<string> errors;
	bytes bytecode = lll::compileLLL(_sourceCode, dev::test::Options::get().evmVersion(), false, &errors);
	if (!errors.empty())
		return false;
	if (bytecode.empty())
		return false;
	return true;
}

}

BOOST_AUTO_TEST_SUITE(LLLCompiler)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* sourceCode = "1";
	BOOST_CHECK(successCompile(sourceCode));
}

BOOST_AUTO_TEST_CASE(switch_valid)
{
	char const* sourceCode = R"(
		(switch (origin))
	)";
	BOOST_CHECK(successCompile(sourceCode));
	sourceCode = R"(
		(switch
			1 (panic)
			2 (panic))
	)";
	BOOST_CHECK(successCompile(sourceCode));
	sourceCode = R"(
		(switch
			1 (panic)
			2 (panic)
			(panic))
	)";
	BOOST_CHECK(successCompile(sourceCode));
	sourceCode = R"(
		(switch
			1 (origin)
			2 (origin)
			(origin))
	)";
	BOOST_CHECK(successCompile(sourceCode));
}

BOOST_AUTO_TEST_CASE(switch_invalid_arg_count)
{
	char const* sourceCode = R"(
		(switch)
	)";
	BOOST_CHECK(!successCompile(sourceCode));
}

BOOST_AUTO_TEST_CASE(switch_inconsistent_return_count)
{
	// cannot return stack items if the default case is not present
	char const* sourceCode = R"(
		(switch
			1 (origin)
			2 (origin)
	)";
	BOOST_CHECK(!successCompile(sourceCode));
	// return count mismatch
	sourceCode = R"(
		(switch
			1 (origin)
			2 (origin)
			(panic))
	)";
	BOOST_CHECK(!successCompile(sourceCode));
	// return count mismatch
	sourceCode = R"(
		(switch
			1 (panic)
			2 (panic)
			(origin))
	)";
	BOOST_CHECK(!successCompile(sourceCode));
}

BOOST_AUTO_TEST_CASE(disallowed_asm_instructions)
{
	for (unsigned i = 1; i <= 32; i++)
		BOOST_CHECK(!successCompile("(asm PUSH" + to_string(i) + ")"));
}

BOOST_AUTO_TEST_CASE(disallowed_functional_asm_instructions)
{
	for (unsigned i = 1; i <= 32; i++)
		BOOST_CHECK(!successCompile("(PUSH" + to_string(i) + ")"));
	for (unsigned i = 1; i <= 16; i++)
		BOOST_CHECK(!successCompile("(DUP" + to_string(i) + ")"));
	for (unsigned i = 1; i <= 16; i++)
		BOOST_CHECK(!successCompile("(SWAP" + to_string(i) + ")"));
	BOOST_CHECK(!successCompile("(JUMPDEST)"));
}

BOOST_AUTO_TEST_CASE(valid_opcodes_functional)
{
	vector<string> opcodes_bytecode {
		"0000",
		"600060000100",
		"600060000200",
		"600060000300",
		"600060000400",
		"600060000500",
		"600060000600",
		"600060000700",
		"6000600060000800",
		"6000600060000900",
		"600060000a00",
		"600060000b00",
		"600060001000",
		"600060001100",
		"600060001200",
		"600060001300",
		"600060001400",
		"60001500",
		"600060001600",
		"600060001700",
		"600060001800",
		"60001900",
		"600060001a00",
		"600060002000",
		"3000",
		"60003100",
		"3200",
		"3300",
		"3400",
		"60003500",
		"3600",
		"6000600060003700",
		"3800",
		"6000600060003900",
		"3a00",
		"60003b00",
		"60006000600060003c00",
		"3d00",
		"6000600060003e00",
		"60003f00",
		"60004000",
		"4100",
		"4200",
		"4300",
		"4400",
		"4500",
		"60005000",
		"60005100",
		"600060005200",
		"600060005300",
		"60005400",
		"600060005500",
		"60005600",
		"600060005700",
		"5800",
		"5900",
		"5a00",
		"60ff00",
		"61ffff00",
		"62ffffff00",
		"63ffffffff00",
		"64ffffffffff00",
		"65ffffffffffff00",
		"66ffffffffffffff00",
		"67ffffffffffffffff00",
		"68ffffffffffffffffff00",
		"69ffffffffffffffffffff00",
		"6affffffffffffffffffffff00",
		"6bffffffffffffffffffffffff00",
		"6cffffffffffffffffffffffffff00",
		"6dffffffffffffffffffffffffffff00",
		"6effffffffffffffffffffffffffffff00",
		"6fffffffffffffffffffffffffffffffff00",
		"70ffffffffffffffffffffffffffffffffff00",
		"71ffffffffffffffffffffffffffffffffffff00",
		"72ffffffffffffffffffffffffffffffffffffff00",
		"73ffffffffffffffffffffffffffffffffffffffff00",
		"74ffffffffffffffffffffffffffffffffffffffffff00",
		"75ffffffffffffffffffffffffffffffffffffffffffff00",
		"76ffffffffffffffffffffffffffffffffffffffffffffff00",
		"77ffffffffffffffffffffffffffffffffffffffffffffffff00",
		"78ffffffffffffffffffffffffffffffffffffffffffffffffff00",
		"79ffffffffffffffffffffffffffffffffffffffffffffffffffff00",
		"7affffffffffffffffffffffffffffffffffffffffffffffffffffff00",
		"7bffffffffffffffffffffffffffffffffffffffffffffffffffffffff00",
		"7cffffffffffffffffffffffffffffffffffffffffffffffffffffffffff00",
		"7dffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff00",
		"7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff00",
		"7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff00",
		"60006000a000",
		"600060006000a100",
		"6000600060006000a200",
		"60006000600060006000a300",
		"600060006000600060006000a400",
		"600060006000f000",
		"6000600060006000600060006000f100",
		"6000600060006000600060006000f200",
		"60006000f300",
		"600060006000600060006000f400",
		"600060006000600060006000fa00",
		"60006000fd00",
		"fe00",
		"6000ff00"
	};

	vector<string> opcodes_lll {
		"(STOP)",
		"(ADD 0 0)",
		"(MUL 0 0)",
		"(SUB 0 0)",
		"(DIV 0 0)",
		"(SDIV 0 0)",
		"(MOD 0 0)",
		"(SMOD 0 0)",
		"(ADDMOD 0 0 0)",
		"(MULMOD 0 0 0)",
		"(EXP 0 0)",
		"(SIGNEXTEND 0 0)",
		"(LT 0 0)",
		"(GT 0 0)",
		"(SLT 0 0)",
		"(SGT 0 0)",
		"(EQ 0 0)",
		"(ISZERO 0)",
		"(AND 0 0)",
		"(OR 0 0)",
		"(XOR 0 0)",
		"(NOT 0)",
		"(BYTE 0 0)",
		"(KECCAK256 0 0)",
		"(ADDRESS)",
		"(BALANCE 0)",
		"(ORIGIN)",
		"(CALLER)",
		"(CALLVALUE)",
		"(CALLDATALOAD 0)",
		"(CALLDATASIZE)",
		"(CALLDATACOPY 0 0 0)",
		"(CODESIZE)",
		"(CODECOPY 0 0 0)",
		"(GASPRICE)",
		"(EXTCODESIZE 0)",
		"(EXTCODECOPY 0 0 0 0)",
		"(RETURNDATASIZE)",
		"(RETURNDATACOPY 0 0 0)",
		"(EXTCODEHASH 0)",
		"(BLOCKHASH 0)",
		"(COINBASE)",
		"(TIMESTAMP)",
		"(NUMBER)",
		"(DIFFICULTY)",
		"(GASLIMIT)",
		"(POP 0)",
		"(MLOAD 0)",
		"(MSTORE 0 0)",
		"(MSTORE8 0 0)",
		"(SLOAD 0)",
		"(SSTORE 0 0)",
		"(JUMP 0)",
		"(JUMPI 0 0)",
		"(PC)",
		"(MSIZE)",
		"(GAS)",
		"0xff",
		"0xffff",
		"0xffffff",
		"0xffffffff",
		"0xffffffffff",
		"0xffffffffffff",
		"0xffffffffffffff",
		"0xffffffffffffffff",
		"0xffffffffffffffffff",
		"0xffffffffffffffffffff",
		"0xffffffffffffffffffffff",
		"0xffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"(LOG0 0 0)",
		"(LOG1 0 0 0)",
		"(LOG2 0 0 0 0)",
		"(LOG3 0 0 0 0 0)",
		"(LOG4 0 0 0 0 0 0)",
		"(CREATE 0 0 0)",
		"(CALL 0 0 0 0 0 0 0)",
		"(CALLCODE 0 0 0 0 0 0 0)",
		"(RETURN 0 0)",
		"(DELEGATECALL 0 0 0 0 0 0)",
		"(STATICCALL 0 0 0 0 0 0)",
		"(REVERT 0 0)",
		"(INVALID)",
		"(SELFDESTRUCT 0)"
	};

	for (size_t i = 0; i < opcodes_bytecode.size(); i++)
	{
		vector<string> errors;
		bytes code = lll::compileLLL(opcodes_lll[i], dev::test::Options::get().evmVersion(), false, &errors);

		BOOST_REQUIRE_MESSAGE(errors.empty(), opcodes_lll[i]);

		BOOST_CHECK_EQUAL(toHex(code), opcodes_bytecode[i]);
	}
}

BOOST_AUTO_TEST_CASE(valid_opcodes_asm)
{
	vector<string> opcodes_bytecode {
		"0000",
		"600060000100",
		"600060000200",
		"600060000300",
		"600060000400",
		"600060000500",
		"600060000600",
		"600060000700",
		"6000600060000800",
		"6000600060000900",
		"600060000a00",
		"600060000b00",
		"600060001000",
		"600060001100",
		"600060001200",
		"600060001300",
		"600060001400",
		"60001500",
		"600060001600",
		"600060001700",
		"600060001800",
		"60001900",
		"600060001a00",
		"600060002000",
		"3000",
		"60003100",
		"3200",
		"3300",
		"3400",
		"60003500",
		"3600",
		"6000600060003700",
		"3800",
		"6000600060003900",
		"3a00",
		"60003b00",
		"60006000600060003c00",
		"3d00",
		"6000600060003e00",
		"60003f00",
		"4000",
		"4100",
		"4200",
		"4300",
		"4400",
		"4500",
		"60005000",
		"60005100",
		"600060005200",
		"600060005300",
		"60005400",
		"600060005500",
		"60005600",
		"600060005700",
		"5800",
		"5900",
		"5a00",
		"5b00",
		"60ff00",
		"61ffff00",
		"62ffffff00",
		"63ffffffff00",
		"64ffffffffff00",
		"65ffffffffffff00",
		"66ffffffffffffff00",
		"67ffffffffffffffff00",
		"68ffffffffffffffffff00",
		"69ffffffffffffffffffff00",
		"6affffffffffffffffffffff00",
		"6bffffffffffffffffffffffff00",
		"6cffffffffffffffffffffffffff00",
		"6dffffffffffffffffffffffffffff00",
		"6effffffffffffffffffffffffffffff00",
		"6fffffffffffffffffffffffffffffffff00",
		"70ffffffffffffffffffffffffffffffffff00",
		"71ffffffffffffffffffffffffffffffffffff00",
		"72ffffffffffffffffffffffffffffffffffffff00",
		"73ffffffffffffffffffffffffffffffffffffffff00",
		"74ffffffffffffffffffffffffffffffffffffffffff00",
		"75ffffffffffffffffffffffffffffffffffffffffffff00",
		"76ffffffffffffffffffffffffffffffffffffffffffffff00",
		"77ffffffffffffffffffffffffffffffffffffffffffffffff00",
		"78ffffffffffffffffffffffffffffffffffffffffffffffffff00",
		"79ffffffffffffffffffffffffffffffffffffffffffffffffffff00",
		"7affffffffffffffffffffffffffffffffffffffffffffffffffffff00",
		"7bffffffffffffffffffffffffffffffffffffffffffffffffffffffff00",
		"7cffffffffffffffffffffffffffffffffffffffffffffffffffffffffff00",
		"7dffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff00",
		"7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff00",
		"7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff00",
		"60006000600060006000600060006000600060006000600060006000600060008000",
		"60006000600060006000600060006000600060006000600060006000600060008100",
		"60006000600060006000600060006000600060006000600060006000600060008200",
		"60006000600060006000600060006000600060006000600060006000600060008300",
		"60006000600060006000600060006000600060006000600060006000600060008400",
		"60006000600060006000600060006000600060006000600060006000600060008500",
		"60006000600060006000600060006000600060006000600060006000600060008600",
		"60006000600060006000600060006000600060006000600060006000600060008700",
		"60006000600060006000600060006000600060006000600060006000600060008800",
		"60006000600060006000600060006000600060006000600060006000600060008900",
		"60006000600060006000600060006000600060006000600060006000600060008a00",
		"60006000600060006000600060006000600060006000600060006000600060008b00",
		"60006000600060006000600060006000600060006000600060006000600060008c00",
		"60006000600060006000600060006000600060006000600060006000600060008d00",
		"60006000600060006000600060006000600060006000600060006000600060008e00",
		"60006000600060006000600060006000600060006000600060006000600060008f00",
		"60006000600060006000600060006000600060006000600060006000600060009000",
		"60006000600060006000600060006000600060006000600060006000600060009100",
		"60006000600060006000600060006000600060006000600060006000600060009200",
		"60006000600060006000600060006000600060006000600060006000600060009300",
		"60006000600060006000600060006000600060006000600060006000600060009400",
		"60006000600060006000600060006000600060006000600060006000600060009500",
		"60006000600060006000600060006000600060006000600060006000600060009600",
		"60006000600060006000600060006000600060006000600060006000600060009700",
		"60006000600060006000600060006000600060006000600060006000600060009800",
		"60006000600060006000600060006000600060006000600060006000600060009900",
		"60006000600060006000600060006000600060006000600060006000600060009a00",
		"60006000600060006000600060006000600060006000600060006000600060009b00",
		"60006000600060006000600060006000600060006000600060006000600060009c00",
		"60006000600060006000600060006000600060006000600060006000600060009d00",
		"60006000600060006000600060006000600060006000600060006000600060009e00",
		"60006000600060006000600060006000600060006000600060006000600060009f00",
		"60006000a000",
		"600060006000a100",
		"6000600060006000a200",
		"60006000600060006000a300",
		"600060006000600060006000a400",
		"600060006000f000",
		"600060006000600060006000f100",
		"600060006000600060006000f200",
		"60006000f300",
		"60006000600060006000f400",
		"60006000600060006000fa00",
		"60006000fd00",
		"fe00",
		"6000ff00"
	};

	vector<string> opcodes_lll {
		"(asm STOP)",
		"(asm 0 0 ADD)",
		"(asm 0 0 MUL)",
		"(asm 0 0 SUB)",
		"(asm 0 0 DIV)",
		"(asm 0 0 SDIV)",
		"(asm 0 0 MOD)",
		"(asm 0 0 SMOD)",
		"(asm 0 0 0 ADDMOD)",
		"(asm 0 0 0 MULMOD)",
		"(asm 0 0 EXP)",
		"(asm 0 0 SIGNEXTEND)",
		"(asm 0 0 LT)",
		"(asm 0 0 GT)",
		"(asm 0 0 SLT)",
		"(asm 0 0 SGT)",
		"(asm 0 0 EQ)",
		"(asm 0 ISZERO)",
		"(asm 0 0 AND)",
		"(asm 0 0 OR)",
		"(asm 0 0 XOR)",
		"(asm 0 NOT)",
		"(asm 0 0 BYTE)",
		"(asm 0 0 KECCAK256)",
		"(asm ADDRESS)",
		"(asm 0 BALANCE)",
		"(asm ORIGIN)",
		"(asm CALLER)",
		"(asm CALLVALUE)",
		"(asm 0 CALLDATALOAD)",
		"(asm CALLDATASIZE)",
		"(asm 0 0 0 CALLDATACOPY)",
		"(asm CODESIZE)",
		"(asm 0 0 0 CODECOPY)",
		"(asm GASPRICE)",
		"(asm 0 EXTCODESIZE)",
		"(asm 0 0 0 0 EXTCODECOPY)",
		"(asm RETURNDATASIZE)",
		"(asm 0 0 0 RETURNDATACOPY)",
		"(asm 0 EXTCODEHASH)",
		"(asm BLOCKHASH)",
		"(asm COINBASE)",
		"(asm TIMESTAMP)",
		"(asm NUMBER)",
		"(asm DIFFICULTY)",
		"(asm GASLIMIT)",
		"(asm 0 POP)",
		"(asm 0 MLOAD)",
		"(asm 0 0 MSTORE)",
		"(asm 0 0 MSTORE8)",
		"(asm 0 SLOAD)",
		"(asm 0 0 SSTORE)",
		"(asm 0 JUMP)",
		"(asm 0 0 JUMPI)",
		"(asm PC)",
		"(asm MSIZE)",
		"(asm GAS)",
		"(asm JUMPDEST)",
		"(asm 0xff)",
		"(asm 0xffff)",
		"(asm 0xffffff)",
		"(asm 0xffffffff)",
		"(asm 0xffffffffff)",
		"(asm 0xffffffffffff)",
		"(asm 0xffffffffffffff)",
		"(asm 0xffffffffffffffff)",
		"(asm 0xffffffffffffffffff)",
		"(asm 0xffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)",
		"(asm 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 DUP1)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 DUP2)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 DUP3)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 DUP4)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 DUP5)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 DUP6)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 DUP7)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 DUP8)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 DUP9)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 DUP10)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 DUP11)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 DUP12)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 DUP13)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 DUP14)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 DUP15)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 DUP16)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 SWAP1)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 SWAP2)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 SWAP3)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 SWAP4)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 SWAP5)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 SWAP6)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 SWAP7)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 SWAP8)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 SWAP9)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 SWAP10)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 SWAP11)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 SWAP12)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 SWAP13)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 SWAP14)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 SWAP15)",
		"(asm 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 SWAP16)",
		"(asm 0 0 LOG0)",
		"(asm 0 0 0 LOG1)",
		"(asm 0 0 0 0 LOG2)",
		"(asm 0 0 0 0 0 LOG3)",
		"(asm 0 0 0 0 0 0 LOG4)",
		"(asm 0 0 0 CREATE)",
		"(asm 0 0 0 0 0 0 CALL)",
		"(asm 0 0 0 0 0 0 CALLCODE)",
		"(asm 0 0 RETURN)",
		"(asm 0 0 0 0 0 DELEGATECALL)",
		"(asm 0 0 0 0 0 STATICCALL)",
		"(asm 0 0 REVERT)",
		"(asm INVALID)",
		"(asm 0 SELFDESTRUCT)"
	};

	for (size_t i = 0; i < opcodes_bytecode.size(); i++)
	{
		vector<string> errors;
		bytes code = lll::compileLLL(opcodes_lll[i], dev::test::Options::get().evmVersion(), false, &errors);

		BOOST_REQUIRE_MESSAGE(errors.empty(), opcodes_lll[i]);

		BOOST_CHECK_EQUAL(toHex(code), opcodes_bytecode[i]);
	}
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
