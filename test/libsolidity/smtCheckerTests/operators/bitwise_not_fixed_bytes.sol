pragma experimental SMTChecker;

contract C {
	function f() public pure {
		// ffff0000 in bytes4
		bytes4 x = ~bytes4(hex"ffff");
		assert(x == 0xffff0000); // should fail
		assert(x == 0x0000ffff); // should hold
	}
}
// ----
// Warning 6328: (133-156): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
