pragma experimental SMTChecker;

contract C {
	function f() public pure {
		assert(bytes4(hex"0000ffff") == bytes4(hex"ffff")); // should fail
		assert(bytes4(hex"ffff0000") == bytes4(hex"ffff")); // should hold
	}
}
// ----
// Warning 6328: (76-126): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
