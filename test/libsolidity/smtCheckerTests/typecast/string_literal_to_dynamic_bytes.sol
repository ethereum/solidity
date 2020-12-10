pragma experimental SMTChecker;

contract C {
	function f() public pure {
		bytes memory b = bytes(hex"ffff");
		assert(b.length == 2); // should hold
		assert(b[0] == byte(uint8(255))); // should hold
		assert(b[1] == byte(uint8(100))); // should fail
	}
}
// ----
// Warning 6328: (204-236): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
