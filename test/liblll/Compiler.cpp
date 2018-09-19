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
		"0100",
		"0200",
		"0300",
		"0400",
		"0500",
		"0600",
		"0700",
		"0800",
		"0900",
		"0a00",
		"0b00",
		"1000",
		"1100",
		"1200",
		"1300",
		"1400",
		"1500",
		"1600",
		"1700",
		"1800",
		"1900",
		"1a00",
		"2000",
		"3000",
		"3100",
		"3200",
		"3300",
		"3400",
		"3500",
		"3600",
		"3700",
		"3800",
		"3900",
		"3a00",
		"3b00",
		"3c00",
		"3d00",
		"3e00",
		"3f00",
		"4000",
		"4100",
		"4200",
		"4300",
		"4400",
		"4500",
		"5000",
		"5100",
		"5200",
		"5300",
		"5400",
		"5500",
		"5600",
		"5700",
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
		"8000",
		"8100",
		"8200",
		"8300",
		"8400",
		"8500",
		"8600",
		"8700",
		"8800",
		"8900",
		"8a00",
		"8b00",
		"8c00",
		"8d00",
		"8e00",
		"8f00",
		"9000",
		"9100",
		"9200",
		"9300",
		"9400",
		"9500",
		"9600",
		"9700",
		"9800",
		"9900",
		"9a00",
		"9b00",
		"9c00",
		"9d00",
		"9e00",
		"9f00",
		"a000",
		"a100",
		"a200",
		"a300",
		"a400",
		"f000",
		"f100",
		"f200",
		"f300",
		"f400",
		"fa00",
		"fd00",
		"fe00",
		"ff00"
	};

	vector<string> opcodes_lll {
		"(asm STOP)",
		"(asm ADD)",
		"(asm MUL)",
		"(asm SUB)",
		"(asm DIV)",
		"(asm SDIV )",
		"(asm MOD)",
		"(asm SMOD)",
		"(asm ADDMOD)",
		"(asm MULMOD)",
		"(asm EXP)",
		"(asm SIGNEXTEND)",
		"(asm LT)",
		"(asm GT)",
		"(asm SLT)",
		"(asm SGT)",
		"(asm EQ)",
		"(asm ISZERO)",
		"(asm AND)",
		"(asm OR)",
		"(asm XOR)",
		"(asm NOT)",
		"(asm BYTE)",
		"(asm KECCAK256)",
		"(asm ADDRESS)",
		"(asm BALANCE)",
		"(asm ORIGIN)",
		"(asm CALLER)",
		"(asm CALLVALUE)",
		"(asm CALLDATALOAD)",
		"(asm CALLDATASIZE)",
		"(asm CALLDATACOPY)",
		"(asm CODESIZE)",
		"(asm CODECOPY)",
		"(asm GASPRICE)",
		"(asm EXTCODESIZE)}",
		"(asm EXTCODECOPY)",
		"(asm RETURNDATASIZE)",
		"(asm RETURNDATACOPY)",
		"(asm EXTCODEHASH)",
		"(asm BLOCKHASH)",
		"(asm COINBASE)",
		"(asm TIMESTAMP)",
		"(asm NUMBER)",
		"(asm DIFFICULTY)",
		"(asm GASLIMIT)",
		"(asm POP)",
		"(asm MLOAD)",
		"(asm MSTORE)",
		"(asm MSTORE8)",
		"(asm SLOAD)",
		"(asm SSTORE)",
		"(asm JUMP )",
		"(asm JUMPI )",
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
		"(asm DUP1)",
		"(asm DUP2)",
		"(asm DUP3)",
		"(asm DUP4)",
		"(asm DUP5)",
		"(asm DUP6)",
		"(asm DUP7)",
		"(asm DUP8)",
		"(asm DUP9)",
		"(asm DUP10)",
		"(asm DUP11)",
		"(asm DUP12)",
		"(asm DUP13)",
		"(asm DUP14)",
		"(asm DUP15)",
		"(asm DUP16)",
		"(asm SWAP1)",
		"(asm SWAP2)",
		"(asm SWAP3)",
		"(asm SWAP4)",
		"(asm SWAP5)",
		"(asm SWAP6)",
		"(asm SWAP7)",
		"(asm SWAP8)",
		"(asm SWAP9)",
		"(asm SWAP10)",
		"(asm SWAP11)",
		"(asm SWAP12)",
		"(asm SWAP13)",
		"(asm SWAP14)",
		"(asm SWAP15)",
		"(asm SWAP16)",
		"(asm LOG0)",
		"(asm LOG1)",
		"(asm LOG2)",
		"(asm LOG3)",
		"(asm LOG4)",
		"(asm CREATE)",
		"(asm CALL)",
		"(asm CALLCODE)",
		"(asm RETURN)",
		"(asm DELEGATECALL)",
		"(asm STATICCALL)",
		"(asm REVERT)",
		"(asm INVALID)",
		"(asm SELFDESTRUCT)"
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
