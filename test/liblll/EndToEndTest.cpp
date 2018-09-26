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
 * @date 2016
 * End to end tests for LLL.
 */

#include <test/liblll/ExecutionFramework.h>
#include <test/Options.h>

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

BOOST_FIXTURE_TEST_SUITE(LLLEndToEndTest, LLLExecutionFramework)

BOOST_AUTO_TEST_CASE(smoke_test)
{
        char const* sourceCode = "(returnlll { (return \"test\") })";
        compileAndRun(sourceCode);
        BOOST_CHECK(callFallback() == encodeArgs(string("test", 4)));
}

BOOST_AUTO_TEST_CASE(bare_panic)
{
	char const* sourceCode = "(panic)";
	compileAndRunWithoutCheck(sourceCode);
	BOOST_REQUIRE(!m_transactionSuccessful);
	BOOST_REQUIRE(m_output.empty());
}

BOOST_AUTO_TEST_CASE(panic)
{
	char const* sourceCode = "{ (panic) }";
	compileAndRunWithoutCheck(sourceCode);
	BOOST_REQUIRE(!m_transactionSuccessful);
	BOOST_REQUIRE(m_output.empty());
}

BOOST_AUTO_TEST_CASE(macro_zeroarg)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(def 'zeroarg () (seq (mstore 0 0x1234) (return 0 32)))
				(zeroarg)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(m_transactionSuccessful);
	BOOST_CHECK(callFallback() == encodeArgs(u256(0x1234)));
}

BOOST_AUTO_TEST_CASE(macros)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(def 'x 1)
				(def 'y () { (def 'x (+ x 2)) })
				(y)
				(return x)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(3)));
}

BOOST_AUTO_TEST_CASE(variables)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(set 'x 1)
				(set 'y 2)
				;; this should equal to 3
				(set 'z (add (get 'x) (get 'y)))
				;; overwriting it here
				(set 'y 4)
				;; each variable has a 32 byte slot, starting from memory location 0x80
				;; variable addresses can also be retrieved by x or (ref 'x)
				(set 'k (add (add (ref 'x) (ref 'y)) z))
				(return (add (add (get 'x) (add (get 'y) (get 'z))) (get 'k)))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(488)));
}

BOOST_AUTO_TEST_CASE(when)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(when (= (calldatasize) 0) (return 1))
				(when (!= (calldatasize) 0) (return 2))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("test()") == encodeArgs(u256(2)));
	BOOST_CHECK(callFallback() == toBigEndian(u256(1)));
}

BOOST_AUTO_TEST_CASE(unless)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(unless (!= (calldatasize) 0) (return 1))
				(unless (= (calldatasize) 0) (return 2))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("test()") == encodeArgs(u256(2)));
	BOOST_CHECK(callFallback() == toBigEndian(u256(1)));
}

BOOST_AUTO_TEST_CASE(conditional_literal)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(return (if (= (calldatasize) 0) 1 2))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("test()") == encodeArgs(u256(2)));
	BOOST_CHECK(callFallback() == toBigEndian(u256(1)));
}

BOOST_AUTO_TEST_CASE(conditional)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(if (= (calldatasize) 0) (return 1) (return 2))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("test()") == encodeArgs(u256(2)));
	BOOST_CHECK(callFallback() == toBigEndian(u256(1)));
}

BOOST_AUTO_TEST_CASE(conditional_seq)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(return (if (= (calldatasize) 0) { 0 2 1 } { 0 1 2 }))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("test()") == encodeArgs(u256(2)));
	BOOST_CHECK(callFallback() == toBigEndian(u256(1)));
}

BOOST_AUTO_TEST_CASE(conditional_nested_else)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(def 'input (calldataload 0x04))
				;; Calculates width in bytes of utf-8 characters.
				(return
					(if (< input 0x80) 1
						(if (< input 0xE0) 2
							(if (< input 0xF0) 3
								(if (< input 0xF8) 4
									(if (< input 0xFC) 5
										6))))))))

	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("test()", 0x00) == encodeArgs(u256(1)));
	BOOST_CHECK(callContractFunction("test()", 0x80) == encodeArgs(u256(2)));
	BOOST_CHECK(callContractFunction("test()", 0xe0) == encodeArgs(u256(3)));
	BOOST_CHECK(callContractFunction("test()", 0xf0) == encodeArgs(u256(4)));
	BOOST_CHECK(callContractFunction("test()", 0xf8) == encodeArgs(u256(5)));
	BOOST_CHECK(callContractFunction("test()", 0xfc) == encodeArgs(u256(6)));
}

BOOST_AUTO_TEST_CASE(conditional_nested_then)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(def 'input (calldataload 0x04))
				;; Calculates width in bytes of utf-8 characters.
				(return
					(if (>= input 0x80)
						(if (>= input 0xE0)
							(if (>= input 0xF0)
								(if (>= input 0xF8)
									(if (>= input 0xFC)
										6 5) 4) 3) 2) 1))))

	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("test()", 0x00) == encodeArgs(u256(1)));
	BOOST_CHECK(callContractFunction("test()", 0x80) == encodeArgs(u256(2)));
	BOOST_CHECK(callContractFunction("test()", 0xe0) == encodeArgs(u256(3)));
	BOOST_CHECK(callContractFunction("test()", 0xf0) == encodeArgs(u256(4)));
	BOOST_CHECK(callContractFunction("test()", 0xf8) == encodeArgs(u256(5)));
	BOOST_CHECK(callContractFunction("test()", 0xfc) == encodeArgs(u256(6)));
}

BOOST_AUTO_TEST_CASE(conditional_switch)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(def 'input (calldataload 0x04))
				;; Calculates width in bytes of utf-8 characters.
				(return
					(switch
						(< input 0x80) 1
						(< input 0xE0) 2
						(< input 0xF0) 3
						(< input 0xF8) 4
						(< input 0xFC) 5
						6))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("test()", 0x00) == encodeArgs(u256(1)));
	BOOST_CHECK(callContractFunction("test()", 0x80) == encodeArgs(u256(2)));
	BOOST_CHECK(callContractFunction("test()", 0xe0) == encodeArgs(u256(3)));
	BOOST_CHECK(callContractFunction("test()", 0xf0) == encodeArgs(u256(4)));
	BOOST_CHECK(callContractFunction("test()", 0xf8) == encodeArgs(u256(5)));
	BOOST_CHECK(callContractFunction("test()", 0xfc) == encodeArgs(u256(6)));
}

BOOST_AUTO_TEST_CASE(conditional_switch_one_arg_with_deposit)
{
	char const* sourceCode = R"(
		(returnlll
			(return
				(switch 42)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(42)));
}

BOOST_AUTO_TEST_CASE(conditional_switch_one_arg_no_deposit)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(switch [0]:42)
				(return 0x00 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(42)));
}

BOOST_AUTO_TEST_CASE(conditional_switch_two_args)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(switch (= (calldataload 0x04) 1) [0]:42)
				(return 0x00 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("test()", 0) == encodeArgs(u256(0)));
	BOOST_CHECK(callContractFunction("test()", 1) == encodeArgs(u256(42)));
}

BOOST_AUTO_TEST_CASE(conditional_switch_three_args_with_deposit)
{
	char const* sourceCode = R"(
		(returnlll
			(return
				(switch (= (calldataload 0x04) 1) 41 42)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("test()", 0) == encodeArgs(u256(42)));
	BOOST_CHECK(callContractFunction("test()", 1) == encodeArgs(u256(41)));
}

BOOST_AUTO_TEST_CASE(conditional_switch_three_args_no_deposit)
{
	char const* sourceCode = R"(
		(returnlll
			(switch
				(= (calldataload 0x04) 1) (return 41)
				(return 42)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("test()", 0) == encodeArgs(u256(42)));
	BOOST_CHECK(callContractFunction("test()", 1) == encodeArgs(u256(41)));
}

BOOST_AUTO_TEST_CASE(exp_operator_const)
{
	char const* sourceCode = R"(
		(returnlll
			(return (exp 2 3)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == toBigEndian(u256(8)));
}

BOOST_AUTO_TEST_CASE(exp_operator_const_signed)
{
	char const* sourceCode = R"(
		(returnlll
			(return (exp (- 0 2) 3)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == toBigEndian(u256(-8)));
}

BOOST_AUTO_TEST_CASE(exp_operator_on_range)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(when (= (div (calldataload 0x00) (exp 2 224)) 0xb3de648b)
					(return (exp 2 (calldataload 0x04))))
				(jump 0x02)))
	)";
	compileAndRun(sourceCode);
	testContractAgainstCppOnRange("f(uint256)", [](u256 const& a) -> u256 { return u256(1 << a.convert_to<int>()); }, 0, 16);
}

BOOST_AUTO_TEST_CASE(constructor_argument_internal_numeric)
{
	char const* sourceCode = R"(
		(seq
			(sstore 0x00 65535)
			(returnlll
				(return @@0x00)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(65535)));
}

BOOST_AUTO_TEST_CASE(constructor_argument_internal_string)
{
	char const* sourceCode = R"(
		(seq
			(sstore 0x00 "test")
			(returnlll
				(return @@0x00)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs("test"));
}

BOOST_AUTO_TEST_CASE(constructor_arguments_external)
{
	char const* sourceCode = R"(
		(seq
			(codecopy 0x00 (bytecodesize) 64)
			(sstore 0x00 @0x00)
			(sstore 0x01 @0x20)
			(returnlll
				(seq
					(when (= (div (calldataload 0x00) (exp 2 224)) 0xf2c9ecd8)
						(return @@0x00))
					(when (= (div (calldataload 0x00) (exp 2 224)) 0x89ea642f)
						(return @@0x01)))))
	)";
	compileAndRun(sourceCode, 0, "", encodeArgs(u256(65535), "test"));
	BOOST_CHECK(callContractFunction("getNumber()") == encodeArgs(u256(65535)));
	BOOST_CHECK(callContractFunction("getString()") == encodeArgs("test"));
}

BOOST_AUTO_TEST_CASE(fallback_and_invalid_function)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(when (= (div (calldataload 0x00) (exp 2 224)) 0xab5ed150)
					(return "one"))
				(when (= (div (calldataload 0x00) (exp 2 224)) 0xee784123)
					(return "two"))
				(return "three")))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("getOne()") == encodeArgs("one"));
	BOOST_CHECK(callContractFunction("getTwo()") == encodeArgs("two"));
	BOOST_CHECK(callContractFunction("invalidFunction()") == encodeArgs("three"));
	BOOST_CHECK(callFallback() == encodeArgs("three"));
}

BOOST_AUTO_TEST_CASE(lit_string)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(lit 0x00 "abcdef")
				(return 0x00 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(string("abcdef")));
}

BOOST_AUTO_TEST_CASE(arithmetic)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(mstore8 0x00 (+ 160 22))
				(mstore8 0x01 (- 223 41))
				(mstore8 0x02 (* 33 2))
				(mstore8 0x03 (/ 10 2))
				(mstore8 0x04 (% 67 2))
				(mstore8 0x05 (& 15 8))
				(mstore8 0x06 (| 18 8))
				(mstore8 0x07 (^ 26 6))
				(return 0x00 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("b6b6420501081a1c000000000000000000000000000000000000000000000000")));
}

BOOST_AUTO_TEST_CASE(binary)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(mstore8 0x00 (< 53 87))
				(mstore8 0x01 (< 73 42))
				(mstore8 0x02 (<= 37 94))
				(mstore8 0x03 (<= 37 37))
				(mstore8 0x04 (<= 183 34))
				(mstore8 0x05 (S< (- 0 53) 87))
				(mstore8 0x06 (S< 73 (- 0 42)))
				(mstore8 0x07 (S<= (- 0 37) 94))
				(mstore8 0x08 (S<= (- 0 37) (- 0 37)))
				(mstore8 0x09 (S<= 183 (- 0 34)))
				(mstore8 0x0a (> 73 42))
				(mstore8 0x0b (> 53 87))
				(mstore8 0x0c (>= 94 37))
				(mstore8 0x0d (>= 94 94))
				(mstore8 0x0e (>= 34 183))
				(mstore8 0x0f (S> 73 (- 0 42)))
				(mstore8 0x10 (S> (- 0 53) 87))
				(mstore8 0x11 (S>= 94 (- 0 37)))
				(mstore8 0x12 (S>= (- 0 94) (- 0 94)))
				(mstore8 0x13 (S>= (- 0 34) 183))
				(mstore8 0x14 (= 53 53))
				(mstore8 0x15 (= 73 42))
				(mstore8 0x16 (!= 37 94))
				(mstore8 0x17 (!= 37 37))
				(return 0x00 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("0100010100010001010001000101000100010100010001000000000000000000")));
}

BOOST_AUTO_TEST_CASE(unary)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(mstore8 0x00 (! (< 53 87)))
				(mstore8 0x01 (! (>= 42 73)))
				(mstore8 0x02 (~ 0x7f))
				(mstore8 0x03 (~ 0xaa))
				(return 0x00 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("0001805500000000000000000000000000000000000000000000000000000000")));
}

BOOST_AUTO_TEST_CASE(assembly_mload_mstore)
{
	char const* sourceCode = R"(
		(returnlll
			(asm
				0x07 0x00 mstore
				"abcdef" 0x20 mstore
				0x00 mload 0x40 mstore
				0x20 mload 0x60 mstore
				0x40 0x40 return))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(7), string("abcdef")));
}

BOOST_AUTO_TEST_CASE(assembly_sload_sstore)
{
	char const* sourceCode = R"(
		(returnlll
			(asm
				0x07 0x00 sstore
				"abcdef" 0x01 sstore
				0x00 sload 0x00 mstore
				0x01 sload 0x20 mstore
				0x40 0x00 return))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(7), string("abcdef")));
}

BOOST_AUTO_TEST_CASE(assembly_codecopy)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(lit 0x00 "abcdef")
				(asm
					0x06 6 codesize sub 0x20 codecopy
					0x20 0x20 return)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(string("abcdef")));
}

BOOST_AUTO_TEST_CASE(for_loop)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(for
					{ (set 'i 1) (set 'j 1) } ; INIT
					(<= @i 10)                ; PRED
					[i]:(+ @i 1)              ; POST
					[j]:(* @j @i))            ; BODY
				(return j 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(3628800))); // 10!
}

BOOST_AUTO_TEST_CASE(while_loop)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				;; Euclid's GCD algorithm
				(set 'a 1071)
				(set 'b 462)
				(while @b
					[a]:(raw @b [b]:(mod @a @b)))
				(return a 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(21))); // GCD(1071,462)
}

BOOST_AUTO_TEST_CASE(keccak256_32bytes)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(mstore 0x00 0x01)
				(return (keccak256 0x00 0x20))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("b10e2d527612073b26eecdfd717e6a320cf44b4afac2b0732d9fcbe2b7fa0cf6")));
}

// The following tests are for the built-in macros.
// Note that panic, returnlll and return_one_arg are well covered above.

BOOST_AUTO_TEST_CASE(allgas)
{
	char const* sourceCode = R"(
		(returnlll
			(return (- (gas) allgas)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(16))); // == 21 - SUB - GAS
}

BOOST_AUTO_TEST_CASE(send_two_args)
{
	// "send" does not retain enough gas to be able to pay for account creation.
	// Disabling for non-tangerineWhistle VMs.
	if (dev::test::Options::get().evmVersion().canOverchargeGasForCall())
	{
		char const* sourceCode = R"(
			(returnlll
				(send 0xdead 42))
		)";
		compileAndRun(sourceCode);
		callFallbackWithValue(42);
		BOOST_CHECK(balanceAt(Address(0xdead)) == 42);
	}
}

BOOST_AUTO_TEST_CASE(send_three_args)
{
	// "send" does not retain enough gas to be able to pay for account creation.
	// Disabling for non-tangerineWhistle VMs.
	if (dev::test::Options::get().evmVersion().canOverchargeGasForCall())
	{
		char const* sourceCode = R"(
			(returnlll
				(send allgas 0xdead 42))
		)";
		compileAndRun(sourceCode);
		callFallbackWithValue(42);
		BOOST_CHECK(balanceAt(Address(0xdead)) == 42);
	}
}

// Regression test for edge case that previously failed
BOOST_AUTO_TEST_CASE(alloc_zero)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(mstore 0x00 (~ 0))
				(alloc 0)
				(return 0x00 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(-1)));
}

BOOST_AUTO_TEST_CASE(alloc_size)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(mstore 0x00 0) ; reserve space for the result of the alloc
				(mstore 0x00 (alloc (calldataload 0x04)))
				(return (- (msize) (mload 0x00)))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("test()", 0)  == encodeArgs(u256(0)));
	BOOST_CHECK(callContractFunction("test()", 1)  == encodeArgs(u256(32)));
	BOOST_CHECK(callContractFunction("test()", 32) == encodeArgs(u256(32)));
	BOOST_CHECK(callContractFunction("test()", 33) == encodeArgs(u256(64)));
}

BOOST_AUTO_TEST_CASE(alloc_start)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(mstore 0x40 0)     ; Set initial MSIZE to 0x60
				(return (alloc 1))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(96));
}

BOOST_AUTO_TEST_CASE(alloc_with_variable)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(set 'x (alloc 1))
				(mstore8 @x 42)    ; ASCII '*'
				(return @x 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs("*"));
}

BOOST_AUTO_TEST_CASE(msg_six_args)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(when (= 0 (calldatasize))
					(seq
						(mstore 0x40 1)
						(def 'outsize 0x20)
						(return (msg 1000 (address) 42 0x40 0x20 outsize) outsize)))
				(when (= 1 (calldataload 0x00))
					(return (callvalue)))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallbackWithValue(42) == encodeArgs(u256(42)));
}

BOOST_AUTO_TEST_CASE(msg_five_args)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(when (= 0 (calldatasize))
					(seq
						(mstore 0x20 1)
						(mstore 0x40 2)
						(return (msg 1000 (address) 42 0x20 0x40))))
				(when (= 3 (+ (calldataload 0x00) (calldataload 0x20)))
					(return (callvalue)))))

	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallbackWithValue(42) == encodeArgs(u256(42)));
}

BOOST_AUTO_TEST_CASE(msg_four_args)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(when (= 0 (calldatasize))
					(return (msg 1000 (address) 42 0xff)))
				(return (callvalue))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallbackWithValue(42) == encodeArgs(u256(42)));
}

BOOST_AUTO_TEST_CASE(msg_three_args)
{
	// "msg" does not retain enough gas.
	// Disabling for non-tangerineWhistle VMs.
	if (dev::test::Options::get().evmVersion().canOverchargeGasForCall())
	{
		char const* sourceCode = R"(
			(returnlll
				(seq
					(when (= 0 (calldatasize))
						(return (msg (address) 42 0xff)))
					(return (callvalue))))
		)";
		compileAndRun(sourceCode);
		BOOST_CHECK(callFallbackWithValue(42) == encodeArgs(u256(42)));
	}
}

BOOST_AUTO_TEST_CASE(msg_two_args)
{
	// "msg" does not retain enough gas.
	// Disabling for non-tangerineWhistle VMs.
	if (dev::test::Options::get().evmVersion().canOverchargeGasForCall())
	{
		char const* sourceCode = R"(
			(returnlll
				(seq
					(when (= 0 (calldatasize))
						(return (msg (address) 0xff)))
					(return 42)))
		)";
		compileAndRun(sourceCode);
		BOOST_CHECK(callFallback() == encodeArgs(u256(42)));
	}
}

BOOST_AUTO_TEST_CASE(create_one_arg)
{
	// "call" does not retain enough gas.
	// Disabling for non-tangerineWhistle VMs.
	if (dev::test::Options::get().evmVersion().canOverchargeGasForCall())
	{
		char const* sourceCode = R"(
			(returnlll
				(seq
					(call allgas
						(create (returnlll (return 42)))
						0 0 0 0x00 0x20)
					(return 0x00 0x20)))
		)";
		compileAndRun(sourceCode);
		BOOST_CHECK(callFallback() == encodeArgs(u256(42)));
	}
}

BOOST_AUTO_TEST_CASE(create_two_args)
{
	// "call" does not retain enough gas.
	// Disabling for non-tangerineWhistle VMs.
	if (dev::test::Options::get().evmVersion().canOverchargeGasForCall())
	{
		char const* sourceCode = R"(
			(returnlll
				(seq
					(call allgas
						(create 42 (returnlll (return (balance (address)))))
						0 0 0 0x00 0x20)
					(return 0x00 0x20)))
		)";
		compileAndRun(sourceCode);
		BOOST_CHECK(callFallbackWithValue(42) == encodeArgs(u256(42)));
	}
}

BOOST_AUTO_TEST_CASE(sha3_two_args)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(mstore 0x00 0x01)
				(return (sha3 0x00 0x20))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("b10e2d527612073b26eecdfd717e6a320cf44b4afac2b0732d9fcbe2b7fa0cf6")));
}

BOOST_AUTO_TEST_CASE(sha3_one_arg)
{
	char const* sourceCode = R"(
		(returnlll
			(return (sha3 0x01)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("b10e2d527612073b26eecdfd717e6a320cf44b4afac2b0732d9fcbe2b7fa0cf6")));
}

BOOST_AUTO_TEST_CASE(sha3pair)
{
	char const* sourceCode = R"(
		(returnlll
			(return (sha3pair 0x01 0x02)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("0xe90b7bceb6e7df5418fb78d8ee546e97c83a08bbccc01a0644d599ccd2a7c2e0")));
}

BOOST_AUTO_TEST_CASE(sha3trip)
{
	char const* sourceCode = R"(
		(returnlll
			(return (sha3trip 0x01 0x02 0x03)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("0x6e0c627900b24bd432fe7b1f713f1b0744091a646a9fe4a65a18dfed21f2949c")));
}

BOOST_AUTO_TEST_CASE(makeperm) // Covers makeperm (implicit), permcount and perm
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(perm 'x) (x (+ 1 x))
				(perm 'y) (y (+ 10 y))
				(when (= 2 permcount)
					(return (+ x y)))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(11)));
}

BOOST_AUTO_TEST_CASE(ecrecover)
{
	// "ecrecover" does not retain enough gas.
	// Disabling for non-tangerineWhistle VMs.
	if (dev::test::Options::get().evmVersion().canOverchargeGasForCall())
	{
		char const* sourceCode = R"(
			(returnlll
				(return
					(ecrecover
						; Hash of 'hello world'
						0x47173285a8d7341e5e972fc677286384f802f8ef42a5ec5f03bbfa254cb01fad
						; v = 1 + 27
						0x1c
						; r
						0xdebaaa0cddb321b2dcaaf846d39605de7b97e77ba6106587855b9106cb104215
						; s
						0x61a22d94fa8b8a687ff9c911c844d1c016d1a685a9166858f9c7c1bc85128aca)))
		)";
		compileAndRun(sourceCode);
		BOOST_CHECK(callFallback() == encodeArgs(fromHex("0x8743523d96a1b2cbe0c6909653a56da18ed484af")));
	}
}

BOOST_AUTO_TEST_CASE(sha256_two_args)
{
	// "sha256" does not retain enough gas.
	// Disabling for non-tangerineWhistle VMs.
	if (dev::test::Options::get().evmVersion().canOverchargeGasForCall())
	{
		char const* sourceCode = R"(
			(returnlll
				(seq
					(lit 0x20 "abcdefghijklmnopqrstuvwxyzABCDEF")
					(lit 0x40 "GHIJKLMNOPQRSTUVWXYZ0123456789?!")
					(sha256 0x20 0x40)
					(return 0x00 0x20)))
		)";
		compileAndRun(sourceCode);
		BOOST_CHECK(callFallback() == encodeArgs(
			fromHex("0xcf25a9fe3d86ae228c226c81d2d8c64c687cd6dc4586d10d8e7e4e5b6706d429")));
	}
}

BOOST_AUTO_TEST_CASE(ripemd160_two_args)
{
	// "ripemd160" does not retain enough gas.
	// Disabling for non-tangerineWhistle VMs.
	if (dev::test::Options::get().evmVersion().canOverchargeGasForCall())
	{
		char const* sourceCode = R"(
			(returnlll
				(seq
					(lit 0x20 "abcdefghijklmnopqrstuvwxyzABCDEF")
					(lit 0x40 "GHIJKLMNOPQRSTUVWXYZ0123456789?!")
					(ripemd160 0x20 0x40)
					(return 0x00 0x20)))
		)";
		compileAndRun(sourceCode);
		BOOST_CHECK(callFallback() == encodeArgs(
			fromHex("0x36c6b90a49e17d4c1e1b0e634ec74124d9b207da")));
	}
}

BOOST_AUTO_TEST_CASE(sha256_one_arg)
{
	// "sha256" does not retain enough gas.
	// Disabling for non-tangerineWhistle VMs.
	if (dev::test::Options::get().evmVersion().canOverchargeGasForCall())
	{
		char const* sourceCode = R"(
			(returnlll
				(seq
					(sha256 0x6162636465666768696a6b6c6d6e6f707172737475767778797a414243444546)
					(return 0x00 0x20)))
		)";
		compileAndRun(sourceCode);
		BOOST_CHECK(callFallback() == encodeArgs(
			fromHex("0xcfd2f1fad75a1978da0a444883db7251414b139f31f5a04704c291fdb0e175e6")));
	}
}

BOOST_AUTO_TEST_CASE(ripemd160_one_arg)
{
	// "ripemd160" does not retain enough gas.
	// Disabling for non-tangerineWhistle VMs.
	if (dev::test::Options::get().evmVersion().canOverchargeGasForCall())
	{
		char const* sourceCode = R"(
			(returnlll
				(seq
					(ripemd160 0x6162636465666768696a6b6c6d6e6f707172737475767778797a414243444546)
					(return 0x00 0x20)))
		)";
		compileAndRun(sourceCode);
		BOOST_CHECK(callFallback() == encodeArgs(
			fromHex("0xac5ab22e07b0fb80c69b6207902f725e2507e546")));
	}
}

BOOST_AUTO_TEST_CASE(wei_szabo_finney_ether)
{
	char const* sourceCode = R"(
		(returnlll
			(return (+ wei (+ szabo (+ finney ether)))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(1001001000000000001)));
}

BOOST_AUTO_TEST_CASE(shift_left)
{
	char const* sourceCode = R"(
		(returnlll
			(return (shl 1 8)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(256)));
}

BOOST_AUTO_TEST_CASE(shift_right)
{
	char const* sourceCode = R"(
		(returnlll
			(return (shr 65536 8)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(256)));
}

BOOST_AUTO_TEST_CASE(sub_assemblies)
{
	char const* sourceCode = R"(
		(returnlll
			(return (create 0 (returnlll (sstore 1 1)))))
	)";
	compileAndRun(sourceCode);
	bytes ret = callFallback();
	BOOST_REQUIRE(ret.size() == 32);
	u256 rVal = u256(toHex(ret, 2, HexPrefix::Add));
	BOOST_CHECK(rVal != 0);
	BOOST_CHECK(rVal < u256("0x10000000000000000000000000000000000000000"));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
