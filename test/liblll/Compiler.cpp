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
		BOOST_CHECK(!successCompile("(asm PUSH" + boost::lexical_cast<string>(i) + ")"));
}

BOOST_AUTO_TEST_CASE(disallowed_functional_asm_instructions)
{
	for (unsigned i = 1; i <= 32; i++)
		BOOST_CHECK(!successCompile("(PUSH" + boost::lexical_cast<string>(i) + ")"));
	for (unsigned i = 1; i <= 16; i++)
		BOOST_CHECK(!successCompile("(DUP" + boost::lexical_cast<string>(i) + ")"));
	for (unsigned i = 1; i <= 16; i++)
		BOOST_CHECK(!successCompile("(SWAP" + boost::lexical_cast<string>(i) + ")"));
	BOOST_CHECK(!successCompile("(JUMPDEST)"));
}

BOOST_AUTO_TEST_CASE(valid_opcodes_functional)
{
	vector<string> opcodes_bytecode {
		"00",
		"6000600001",
		"6000600002",
		"6000600003",
		"6000600004",
		"6000600005",
		"6000600006",
		"6000600007",
		"60006000600008",
		"60006000600009",
		"600060000a",
		"600060000b",
		"6000600010",
		"6000600011",
		"6000600012",
		"6000600013",
		"6000600014",
		"600015",
		"6000600016",
		"6000600017",
		"6000600018",
		"600019",
		"600060001a",
		"6000600020",
		"30",
		"600031",
		"32",
		"33",
		"34",
		"600035",
		"36",
		"60006000600037",
		"38",
		"60006000600039",
		"3a",
		"60003b",
		"60006000600060003c",
		"3d",
		"6000600060003e",
		"600040",
		"41",
		"42",
		"43",
		"44",
		"45",
		"600050",
		"600051",
		"6000600052",
		"6000600053",
		"600054",
		"6000600055",
		"600056",
		"6000600057",
		"58",
		"59",
		"5a",
		"60ff",
		"61ffff",
		"62ffffff",
		"63ffffffff",
		"64ffffffffff",
		"65ffffffffffff",
		"66ffffffffffffff",
		"67ffffffffffffffff",
		"68ffffffffffffffffff",
		"69ffffffffffffffffffff",
		"6affffffffffffffffffffff",
		"6bffffffffffffffffffffffff",
		"6cffffffffffffffffffffffffff",
		"6dffffffffffffffffffffffffffff",
		"6effffffffffffffffffffffffffffff",
		"6fffffffffffffffffffffffffffffffff",
		"70ffffffffffffffffffffffffffffffffff",
		"71ffffffffffffffffffffffffffffffffffff",
		"72ffffffffffffffffffffffffffffffffffffff",
		"73ffffffffffffffffffffffffffffffffffffffff",
		"74ffffffffffffffffffffffffffffffffffffffffff",
		"75ffffffffffffffffffffffffffffffffffffffffffff",
		"76ffffffffffffffffffffffffffffffffffffffffffffff",
		"77ffffffffffffffffffffffffffffffffffffffffffffffff",
		"78ffffffffffffffffffffffffffffffffffffffffffffffffff",
		"79ffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"7affffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"7bffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"7cffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"7dffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"60006000a0",
		"600060006000a1",
		"6000600060006000a2",
		"60006000600060006000a3",
		"600060006000600060006000a4",
		"600060006000f0",
		"6000600060006000600060006000f1",
		"6000600060006000600060006000f2",
		"60006000f3",
		"600060006000600060006000f4",
		"600060006000600060006000fa",
		"60006000fd",
		"fe",
		"6000ff"
	};

	vector<string> opcodes_lll {
		"{ (STOP) }",
		"{ (ADD 0 0) }",
		"{ (MUL 0 0) }",
		"{ (SUB 0 0) }",
		"{ (DIV 0 0) }",
		"{ (SDIV 0 0) }",
		"{ (MOD 0 0) }",
		"{ (SMOD 0 0) }",
		"{ (ADDMOD 0 0 0) }",
		"{ (MULMOD 0 0 0) }",
		"{ (EXP 0 0) }",
		"{ (SIGNEXTEND 0 0) }",
		"{ (LT 0 0) }",
		"{ (GT 0 0) }",
		"{ (SLT 0 0) }",
		"{ (SGT 0 0) }",
		"{ (EQ 0 0) }",
		"{ (ISZERO 0) }",
		"{ (AND 0 0) }",
		"{ (OR 0 0) }",
		"{ (XOR 0 0) }",
		"{ (NOT 0) }",
		"{ (BYTE 0 0) }",
		"{ (KECCAK256 0 0) }",
		"{ (ADDRESS) }",
		"{ (BALANCE 0) }",
		"{ (ORIGIN) }",
		"{ (CALLER) }",
		"{ (CALLVALUE) }",
		"{ (CALLDATALOAD 0) }",
		"{ (CALLDATASIZE) }",
		"{ (CALLDATACOPY 0 0 0) }",
		"{ (CODESIZE) }",
		"{ (CODECOPY 0 0 0) }",
		"{ (GASPRICE) }",
		"{ (EXTCODESIZE 0) }",
		"{ (EXTCODECOPY 0 0 0 0) }",
		"{ (RETURNDATASIZE) }",
		"{ (RETURNDATACOPY 0 0 0) }",
		"{ (BLOCKHASH 0) }",
		"{ (COINBASE) }",
		"{ (TIMESTAMP) }",
		"{ (NUMBER) }",
		"{ (DIFFICULTY) }",
		"{ (GASLIMIT) }",
		"{ (POP 0) }",
		"{ (MLOAD 0) }",
		"{ (MSTORE 0 0) }",
		"{ (MSTORE8 0 0) }",
		"{ (SLOAD 0) }",
		"{ (SSTORE 0 0) }",
		"{ (JUMP 0) }",
		"{ (JUMPI 0 0) }",
		"{ (PC) }",
		"{ (MSIZE) }",
		"{ (GAS) }",
		"{ 0xff }",
		"{ 0xffff }",
		"{ 0xffffff }",
		"{ 0xffffffff }",
		"{ 0xffffffffff }",
		"{ 0xffffffffffff }",
		"{ 0xffffffffffffff }",
		"{ 0xffffffffffffffff }",
		"{ 0xffffffffffffffffff }",
		"{ 0xffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff }",
		"{ 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff }",
		"{ (LOG0 0 0) }",
		"{ (LOG1 0 0 0) }",
		"{ (LOG2 0 0 0 0) }",
		"{ (LOG3 0 0 0 0 0) }",
		"{ (LOG4 0 0 0 0 0 0) }",
		"{ (CREATE 0 0 0) }",
		"{ (CALL 0 0 0 0 0 0 0) }",
		"{ (CALLCODE 0 0 0 0 0 0 0) }",
		"{ (RETURN 0 0) }",
		"{ (DELEGATECALL 0 0 0 0 0 0) }",
		"{ (STATICCALL 0 0 0 0 0 0) }",
		"{ (REVERT 0 0) }",
		"{ (INVALID) }",
		"{ (SELFDESTRUCT 0) }"
	};

	for (size_t i = 0; i < opcodes_bytecode.size(); i++) {
		vector<string> errors;
		bytes code = lll::compileLLL(opcodes_lll[i], dev::test::Options::get().evmVersion(), false, &errors);

		BOOST_REQUIRE_MESSAGE(errors.empty(), opcodes_lll[i]);

		BOOST_CHECK_EQUAL(toHex(code), opcodes_bytecode[i]);
	}
}

BOOST_AUTO_TEST_CASE(valid_opcodes_asm)
{
	vector<string> opcodes_bytecode {
		"00",
		"01",
		"02",
		"03",
		"04",
		"05",
		"06",
		"07",
		"08",
		"09",
		"0a",
		"0b",
		"10",
		"11",
		"12",
		"13",
		"14",
		"15",
		"16",
		"17",
		"18",
		"19",
		"1a",
		"20",
		"30",
		"31",
		"32",
		"33",
		"34",
		"35",
		"36",
		"37",
		"38",
		"39",
		"3a",
		"3b",
		"3c",
		"3d",
		"3e",
		"40",
		"41",
		"42",
		"43",
		"44",
		"45",
		"50",
		"51",
		"52",
		"53",
		"54",
		"55",
		"56",
		"57",
		"58",
		"59",
		"5a",
		"5b",
		"60ff",
		"61ffff",
		"62ffffff",
		"63ffffffff",
		"64ffffffffff",
		"65ffffffffffff",
		"66ffffffffffffff",
		"67ffffffffffffffff",
		"68ffffffffffffffffff",
		"69ffffffffffffffffffff",
		"6affffffffffffffffffffff",
		"6bffffffffffffffffffffffff",
		"6cffffffffffffffffffffffffff",
		"6dffffffffffffffffffffffffffff",
		"6effffffffffffffffffffffffffffff",
		"6fffffffffffffffffffffffffffffffff",
		"70ffffffffffffffffffffffffffffffffff",
		"71ffffffffffffffffffffffffffffffffffff",
		"72ffffffffffffffffffffffffffffffffffffff",
		"73ffffffffffffffffffffffffffffffffffffffff",
		"74ffffffffffffffffffffffffffffffffffffffffff",
		"75ffffffffffffffffffffffffffffffffffffffffffff",
		"76ffffffffffffffffffffffffffffffffffffffffffffff",
		"77ffffffffffffffffffffffffffffffffffffffffffffffff",
		"78ffffffffffffffffffffffffffffffffffffffffffffffffff",
		"79ffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"7affffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"7bffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"7cffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"7dffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
		"80",
		"81",
		"82",
		"83",
		"84",
		"85",
		"86",
		"87",
		"88",
		"89",
		"8a",
		"8b",
		"8c",
		"8d",
		"8e",
		"8f",
		"90",
		"91",
		"92",
		"93",
		"94",
		"95",
		"96",
		"97",
		"98",
		"99",
		"9a",
		"9b",
		"9c",
		"9d",
		"9e",
		"9f",
		"a0",
		"a1",
		"a2",
		"a3",
		"a4",
		"f0",
		"f1",
		"f2",
		"f3",
		"f4",
		"fa",
		"fd",
		"fe",
		"ff"
	};

	vector<string> opcodes_lll {
		"{ (asm STOP) }",
		"{ (asm ADD) }",
		"{ (asm MUL) }",
		"{ (asm SUB) }",
		"{ (asm DIV) }",
		"{ (asm SDIV ) }",
		"{ (asm MOD) }",
		"{ (asm SMOD) }",
		"{ (asm ADDMOD) }",
		"{ (asm MULMOD) }",
		"{ (asm EXP) }",
		"{ (asm SIGNEXTEND) }",
		"{ (asm LT) }",
		"{ (asm GT) }",
		"{ (asm SLT) }",
		"{ (asm SGT) }",
		"{ (asm EQ) }",
		"{ (asm ISZERO) }",
		"{ (asm AND) }",
		"{ (asm OR) }",
		"{ (asm XOR) }",
		"{ (asm NOT) }",
		"{ (asm BYTE) }",
		"{ (asm KECCAK256) }",
		"{ (asm ADDRESS) }",
		"{ (asm BALANCE) }",
		"{ (asm ORIGIN) }",
		"{ (asm CALLER) }",
		"{ (asm CALLVALUE) }",
		"{ (asm CALLDATALOAD) }",
		"{ (asm CALLDATASIZE) }",
		"{ (asm CALLDATACOPY) }",
		"{ (asm CODESIZE) }",
		"{ (asm CODECOPY) }",
		"{ (asm GASPRICE) }",
		"{ (asm EXTCODESIZE)}",
		"{ (asm EXTCODECOPY) }",
		"{ (asm RETURNDATASIZE) }",
		"{ (asm RETURNDATACOPY) }",
		"{ (asm BLOCKHASH) }",
		"{ (asm COINBASE) }",
		"{ (asm TIMESTAMP) }",
		"{ (asm NUMBER) }",
		"{ (asm DIFFICULTY) }",
		"{ (asm GASLIMIT) }",
		"{ (asm POP) }",
		"{ (asm MLOAD) }",
		"{ (asm MSTORE) }",
		"{ (asm MSTORE8) }",
		"{ (asm SLOAD) }",
		"{ (asm SSTORE) }",
		"{ (asm JUMP ) }",
		"{ (asm JUMPI ) }",
		"{ (asm PC) }",
		"{ (asm MSIZE) }",
		"{ (asm GAS) }",
		"{ (asm JUMPDEST) }",
		"{ (asm 0xff) }",
		"{ (asm 0xffff) }",
		"{ (asm 0xffffff) }",
		"{ (asm 0xffffffff) }",
		"{ (asm 0xffffffffff) }",
		"{ (asm 0xffffffffffff) }",
		"{ (asm 0xffffffffffffff) }",
		"{ (asm 0xffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) }",
		"{ (asm 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) }",
		"{ (asm DUP1) }",
		"{ (asm DUP2) }",
		"{ (asm DUP3) }",
		"{ (asm DUP4) }",
		"{ (asm DUP5) }",
		"{ (asm DUP6) }",
		"{ (asm DUP7) }",
		"{ (asm DUP8) }",
		"{ (asm DUP9) }",
		"{ (asm DUP10) }",
		"{ (asm DUP11) }",
		"{ (asm DUP12) }",
		"{ (asm DUP13) }",
		"{ (asm DUP14) }",
		"{ (asm DUP15) }",
		"{ (asm DUP16) }",
		"{ (asm SWAP1) }",
		"{ (asm SWAP2) }",
		"{ (asm SWAP3) }",
		"{ (asm SWAP4) }",
		"{ (asm SWAP5) }",
		"{ (asm SWAP6) }",
		"{ (asm SWAP7) }",
		"{ (asm SWAP8) }",
		"{ (asm SWAP9) }",
		"{ (asm SWAP10) }",
		"{ (asm SWAP11) }",
		"{ (asm SWAP12) }",
		"{ (asm SWAP13) }",
		"{ (asm SWAP14) }",
		"{ (asm SWAP15) }",
		"{ (asm SWAP16) }",
		"{ (asm LOG0) }",
		"{ (asm LOG1) }",
		"{ (asm LOG2) }",
		"{ (asm LOG3) }",
		"{ (asm LOG4) }",
		"{ (asm CREATE) }",
		"{ (asm CALL) }",
		"{ (asm CALLCODE) }",
		"{ (asm RETURN) }",
		"{ (asm DELEGATECALL) }",
		"{ (asm STATICCALL) }",
		"{ (asm REVERT) }",
		"{ (asm INVALID) }",
		"{ (asm SELFDESTRUCT) }"
	};

	for (size_t i = 0; i < opcodes_bytecode.size(); i++) {
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
