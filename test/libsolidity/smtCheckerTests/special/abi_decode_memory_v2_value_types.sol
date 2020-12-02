pragma experimental SMTChecker;
pragma experimental "ABIEncoderV2";

contract C {
	function f() public pure {
		(uint x1, bool b1) = abi.decode("abc", (uint, bool));
		(uint x2, bool b2) = abi.decode("abc", (uint, bool));
		// False positive until abi.* are implemented as uninterpreted functions.
		assert(x1 == x2);
	}
}
// ----
// Warning 2072: (122-129): Unused local variable.
// Warning 2072: (178-185): Unused local variable.
// Warning 4588: (133-164): Assertion checker does not yet implement this type of function call.
// Warning 4588: (189-220): Assertion checker does not yet implement this type of function call.
// Warning 6328: (300-316): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 4588: (133-164): Assertion checker does not yet implement this type of function call.
// Warning 4588: (189-220): Assertion checker does not yet implement this type of function call.
