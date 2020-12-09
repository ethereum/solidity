pragma experimental SMTChecker;
contract C {
	function f() public pure {
		(uint a1, bytes32 b1, C c1) = abi.decode("abc", (uint, bytes32, C));
		(uint a2, bytes32 b2, C c2) = abi.decode("abc", (uint, bytes32, C));
		// False positive until abi.* are implemented as uninterpreted functions.
		assert(a1 == a2);
		assert(a1 != a2);
	}
}
// ----
// Warning 2072: (85-95): Unused local variable.
// Warning 2072: (97-101): Unused local variable.
// Warning 2072: (156-166): Unused local variable.
// Warning 2072: (168-172): Unused local variable.
// Warning 8364: (139-140): Assertion checker does not yet implement type type(contract C)
// Warning 4588: (105-142): Assertion checker does not yet implement this type of function call.
// Warning 8364: (210-211): Assertion checker does not yet implement type type(contract C)
// Warning 4588: (176-213): Assertion checker does not yet implement this type of function call.
// Warning 6328: (293-309): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (313-329): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 8364: (139-140): Assertion checker does not yet implement type type(contract C)
// Warning 4588: (105-142): Assertion checker does not yet implement this type of function call.
// Warning 8364: (210-211): Assertion checker does not yet implement type type(contract C)
// Warning 4588: (176-213): Assertion checker does not yet implement this type of function call.
