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
 * Unit tests for stack-reusing code generator.
 */

#include <test/Options.h>

#include <libyul/AssemblyStack.h>
#include <libevmasm/Instruction.h>

using namespace std;

namespace yul
{
namespace test
{

namespace
{
string assemble(string const& _input)
{
	dev::solidity::OptimiserSettings settings = dev::solidity::OptimiserSettings::full();
	settings.runYulOptimiser = false;
	settings.optimizeStackAllocation = true;
	AssemblyStack asmStack(langutil::EVMVersion{}, AssemblyStack::Language::StrictAssembly, settings);
	BOOST_REQUIRE_MESSAGE(asmStack.parseAndAnalyze("", _input), "Source did not parse: " + _input);
	return dev::eth::disassemble(asmStack.assemble(AssemblyStack::Machine::EVM).bytecode->bytecode);
}
}

BOOST_AUTO_TEST_SUITE(StackReuseCodegen)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	string out = assemble("{}");
	BOOST_CHECK_EQUAL(out, "");
}

BOOST_AUTO_TEST_CASE(single_var)
{
	string out = assemble("{ let x }");
	BOOST_CHECK_EQUAL(out, "PUSH1 0x0 POP ");
}

BOOST_AUTO_TEST_CASE(single_var_assigned)
{
	string out = assemble("{ let x := 1 }");
	BOOST_CHECK_EQUAL(out, "PUSH1 0x1 POP ");
}

BOOST_AUTO_TEST_CASE(single_var_assigned_plus_code)
{
	string out = assemble("{ let x := 1 mstore(3, 4) }");
	BOOST_CHECK_EQUAL(out, "PUSH1 0x1 POP PUSH1 0x4 PUSH1 0x3 MSTORE ");
}

BOOST_AUTO_TEST_CASE(single_var_assigned_plus_code_and_reused)
{
	string out = assemble("{ let x := 1 mstore(3, 4) pop(mload(x)) }");
	BOOST_CHECK_EQUAL(out, "PUSH1 0x1 PUSH1 0x4 PUSH1 0x3 MSTORE DUP1 MLOAD POP POP ");
}

BOOST_AUTO_TEST_CASE(multi_reuse_single_slot)
{
	string out = assemble("{ let x := 1 x := 6 let y := 2 y := 4 }");
	BOOST_CHECK_EQUAL(out, "PUSH1 0x1 PUSH1 0x6 SWAP1 POP POP PUSH1 0x2 PUSH1 0x4 SWAP1 POP POP ");
}

BOOST_AUTO_TEST_CASE(multi_reuse_single_slot_nested)
{
	string out = assemble("{ let x := 1 x := 6 { let y := 2 y := 4 } }");
	BOOST_CHECK_EQUAL(out, "PUSH1 0x1 PUSH1 0x6 SWAP1 POP POP PUSH1 0x2 PUSH1 0x4 SWAP1 POP POP ");
}

BOOST_AUTO_TEST_CASE(multi_reuse_same_variable_name)
{
	string out = assemble("{ let z := mload(0) { let x := 1 x := 6 z := x } { let x := 2 z := x x := 4 } }");
	BOOST_CHECK_EQUAL(out,
		"PUSH1 0x0 MLOAD "
		"PUSH1 0x1 PUSH1 0x6 SWAP1 POP DUP1 SWAP2 POP POP "
		"PUSH1 0x2 DUP1 SWAP2 POP PUSH1 0x4 SWAP1 POP POP "
		"POP "
	);
}

BOOST_AUTO_TEST_CASE(last_use_in_nested_block)
{
	string out = assemble("{ let z := 0 { pop(z) } let x := 1 }");
	BOOST_CHECK_EQUAL(out, "PUSH1 0x0 DUP1 POP POP PUSH1 0x1 POP ");
}

BOOST_AUTO_TEST_CASE(if_)
{
	// z is only removed after the if (after the jumpdest)
	string out = assemble("{ let z := mload(0) if z { let x := z } let t := 3 }");
	BOOST_CHECK_EQUAL(out, "PUSH1 0x0 MLOAD DUP1 ISZERO PUSH1 0xA JUMPI DUP1 POP JUMPDEST POP PUSH1 0x3 POP ");
}

BOOST_AUTO_TEST_CASE(switch_)
{
	string out = assemble("{ let z := 0 switch z case 0 { let x := 2 let y := 3 } default { z := 3 } let t := 9 }");
	BOOST_CHECK_EQUAL(out,
		"PUSH1 0x0 DUP1 "
		"PUSH1 0x0 DUP2 EQ PUSH1 0x11 JUMPI "
		"PUSH1 0x3 SWAP2 POP PUSH1 0x18 JUMP "
		"JUMPDEST PUSH1 0x2 POP PUSH1 0x3 POP "
		"JUMPDEST POP POP " // This is where z and its copy (switch condition) can be removed.
		"PUSH1 0x9 POP "
	);
}

BOOST_AUTO_TEST_CASE(reuse_slots)
{
	// x and y should reuse the slots of b and d
	string out = assemble("{ let a, b, c, d let x := 2 let y := 3 mstore(x, a) mstore(y, c) }");
	BOOST_CHECK_EQUAL(out,
		"PUSH1 0x0 PUSH1 0x0 PUSH1 0x0 PUSH1 0x0 "
		"POP " // d is removed right away
		"PUSH1 0x2 SWAP2 POP " // x is stored at b's slot
		"PUSH1 0x3 DUP4 DUP4 MSTORE "
		"DUP2 DUP2 MSTORE "
		"POP POP POP POP "
	);
}

BOOST_AUTO_TEST_CASE(for_1)
{
	// Special scoping rules, but can remove z early
	string out = assemble("{ for { let z := 0 } 1 { } { let x := 3 } let t := 2 }");
	BOOST_CHECK_EQUAL(out,
		"PUSH1 0x0 POP "
		"JUMPDEST PUSH1 0x1 ISZERO PUSH1 0x11 JUMPI "
		"PUSH1 0x3 POP JUMPDEST PUSH1 0x3 JUMP "
		"JUMPDEST PUSH1 0x2 POP "
	);
}

BOOST_AUTO_TEST_CASE(for_2)
{
	// Special scoping rules, cannot remove z until after the loop!
	string out = assemble("{ for { let z := 0 } 1 { } { z := 8 let x := 3 } let t := 2 }");
	BOOST_CHECK_EQUAL(out,
		"PUSH1 0x0 "
		"JUMPDEST PUSH1 0x1 ISZERO PUSH1 0x14 JUMPI "
		"PUSH1 0x8 SWAP1 POP "
		"PUSH1 0x3 POP "
		"JUMPDEST PUSH1 0x2 JUMP "
		"JUMPDEST POP " // z is removed
		"PUSH1 0x2 POP "
	);
}

BOOST_AUTO_TEST_CASE(function_trivial)
{
	string in = R"({
		function f() { }
	})";
	BOOST_CHECK_EQUAL(assemble(in),
		"PUSH1 0x5 JUMP JUMPDEST JUMP JUMPDEST "
	);
}

BOOST_AUTO_TEST_CASE(function_retparam)
{
	string in = R"({
		function f() -> x, y { }
	})";
	BOOST_CHECK_EQUAL(assemble(in),
		"PUSH1 0xB JUMP "
		"JUMPDEST PUSH1 0x0 PUSH1 0x0 SWAP1 SWAP2 JUMP "
		"JUMPDEST "
	);
}

BOOST_AUTO_TEST_CASE(function_params)
{
	string in = R"({
		function f(a, b) { }
	})";
	BOOST_CHECK_EQUAL(assemble(in), "PUSH1 0x7 JUMP JUMPDEST POP POP JUMP JUMPDEST ");
}

BOOST_AUTO_TEST_CASE(function_params_and_retparams)
{
	string in = R"({
		function f(a, b, c, d) -> x, y { }
	})";
	// This does not re-use the parameters for the return parameters
	// We do not expect parameters to be fully unused, so the stack
	// layout for a function is still fixed, even though parameters
	// can be re-used.
	BOOST_CHECK_EQUAL(assemble(in),
		"PUSH1 0x10 JUMP JUMPDEST PUSH1 0x0 PUSH1 0x0 SWAP5 POP SWAP5 SWAP3 POP POP POP JUMP JUMPDEST "
	);
}

BOOST_AUTO_TEST_CASE(function_params_and_retparams_partly_unused)
{
	string in = R"({
		function f(a, b, c, d) -> x, y { b := 3 let s := 9 y := 2 mstore(s, y) }
	})";
	BOOST_CHECK_EQUAL(assemble(in),
		"PUSH1 0x1E JUMP "
		"JUMPDEST PUSH1 0x0 PUSH1 0x0 "
		"PUSH1 0x3 SWAP4 POP "
		"PUSH1 0x9 PUSH1 0x2 SWAP2 POP "
		"DUP2 DUP2 MSTORE "
		"POP SWAP5 POP SWAP5 SWAP3 POP POP POP JUMP "
		"JUMPDEST "
	);
}

BOOST_AUTO_TEST_CASE(function_with_body_embedded)
{
	string in = R"({
		let b := 3
		function f(a, r) -> t {
			// r could be removed right away, but a cannot - this is not implemented, though
			let x := a a := 3 t := a
		}
		b := 7
	})";
	BOOST_CHECK_EQUAL(assemble(in),
		"PUSH1 0x3 PUSH1 "
		"0x16 JUMP "
		"JUMPDEST PUSH1 0x0 " // start of f, initialize t
		"DUP2 POP " // let x := a
		"PUSH1 0x3 SWAP2 POP "
		"DUP2 SWAP1 POP "
		"SWAP3 SWAP2 POP POP JUMP "
		"JUMPDEST PUSH1 0x7 SWAP1 "
		"POP POP "
	);
}

BOOST_AUTO_TEST_CASE(function_call)
{
	string in = R"({
		let b := f(1, 2)
		function f(a, r) -> t { }
		b := f(3, 4)
	})";
	BOOST_CHECK_EQUAL(assemble(in),
		"PUSH1 0x9 PUSH1 0x2 PUSH1 0x1 PUSH1 0xD JUMP "
		"JUMPDEST PUSH1 0x15 JUMP " // jump over f
		"JUMPDEST PUSH1 0x0 SWAP3 SWAP2 POP POP JUMP " // f
		"JUMPDEST PUSH1 0x1F PUSH1 0x4 PUSH1 0x3 PUSH1 0xD JUMP "
		"JUMPDEST SWAP1 POP POP "
	);
}


BOOST_AUTO_TEST_CASE(functions_multi_return)
{
	string in = R"({
		function f(a, b) -> t { }
		function g() -> r, s { }
		let x := f(1, 2)
		x := f(3, 4)
		let y, z := g()
		y, z := g()
		let unused := 7
	})";
	BOOST_CHECK_EQUAL(assemble(in),
		"PUSH1 0x13 JUMP "
		"JUMPDEST PUSH1 0x0 SWAP3 SWAP2 POP POP JUMP " // f
		"JUMPDEST PUSH1 0x0 PUSH1 0x0 SWAP1 SWAP2 JUMP " // g
		"JUMPDEST PUSH1 0x1D PUSH1 0x2 PUSH1 0x1 PUSH1 0x3 JUMP " // f(1, 2)
		"JUMPDEST PUSH1 0x27 PUSH1 0x4 PUSH1 0x3 PUSH1 0x3 JUMP " // f(3, 4)
		"JUMPDEST SWAP1 POP " // assignment to x
		"POP " // remove x
		"PUSH1 0x30 PUSH1 0xB JUMP " // g()
		"JUMPDEST PUSH1 0x36 PUSH1 0xB JUMP " // g()
		"JUMPDEST SWAP2 POP SWAP2 POP " // assignments
		"POP POP " // removal of y and z
		"PUSH1 0x7 POP "
	);
}

BOOST_AUTO_TEST_CASE(reuse_slots_function)
{
	string in = R"({
		function f() -> x, y, z, t {}
		let a, b, c, d := f() let x1 := 2 let y1 := 3 mstore(x1, a) mstore(y1, c)
	})";
	BOOST_CHECK_EQUAL(assemble(in),
		"PUSH1 0x11 JUMP "
		"JUMPDEST PUSH1 0x0 PUSH1 0x0 PUSH1 0x0 PUSH1 0x0 SWAP1 SWAP2 SWAP3 SWAP4 JUMP "
		"JUMPDEST PUSH1 0x17 PUSH1 0x3 JUMP "
		// Stack: a b c d
		"JUMPDEST POP " // d is unused
		// Stack: a b c
		"PUSH1 0x2 SWAP2 POP " // x1 reuses b's slot
		"PUSH1 0x3 "
		// Stack: a x1 c y1
		"DUP4 DUP4 MSTORE "
		"DUP2 DUP2 MSTORE "
		"POP POP POP POP "
	);
}

BOOST_AUTO_TEST_CASE(reuse_slots_function_with_gaps)
{
	string in = R"({
		// Only x3 is actually used, the slots of
		// x1 and x2 will be reused right away.
		let x1 := 5 let x2 := 6 let x3 := 7
		mstore(x1, x2)
		function f() -> x, y, z, t {}
		let a, b, c, d := f() mstore(x3, a) mstore(c, d)
	})";
	BOOST_CHECK_EQUAL(assemble(in),
		"PUSH1 0x5 PUSH1 0x6 PUSH1 0x7 "
		"DUP2 DUP4 MSTORE "
		"PUSH1 0x1A JUMP " // jump across function
		"JUMPDEST PUSH1 0x0 PUSH1 0x0 PUSH1 0x0 PUSH1 0x0 SWAP1 SWAP2 SWAP3 SWAP4 JUMP "
		"JUMPDEST PUSH1 0x20 PUSH1 0xC JUMP "
		// stack: x1 x2 x3 a b c d
		"JUMPDEST SWAP6 POP " // move d into x1
		// stack: d x2 x3 a b c
		"SWAP4 POP "
		// stack: d c x3 a b
		"POP "
		// stack: d c x3 a
		"DUP1 DUP3 MSTORE "
		"POP POP "
		// stack: d c
		"DUP2 DUP2 MSTORE "
		"POP POP "
	);
}


BOOST_AUTO_TEST_SUITE_END()

}
}
