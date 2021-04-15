contract C {
	function f() public pure {
		// ffff0000 in bytes4
		bytes4 x = ~bytes4(hex"ffff");
		assert(x == 0xffff0000); // should fail
		assert(x == 0x0000ffff); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (100-123): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 65535\n\nTransaction trace:\nC.constructor()\nC.f()
