pragma experimental SMTChecker;
contract C {
	function f() public pure {
		assert(byte("") & ("") == byte(0)); // should hold
		assert(byte(0xAA) & byte(0x55) == byte(0)); // should hold
		assert(byte(0xFF) & byte(0xAA) == byte(0xAA)); // should hold
		assert(byte(0xFF) & byte(0xAA) == byte(0)); // should fail
	}
}
// ----
// Warning 6328: (253-295): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
