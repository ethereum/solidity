contract C {
	function f() public pure {
		assert(bytes4(hex"0000ffff") == bytes4(hex"ffff")); // should fail
		assert(bytes4(hex"ffff0000") == bytes4(hex"ffff")); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (43-93): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
