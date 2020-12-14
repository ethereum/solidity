pragma experimental SMTChecker;
contract C {
	function f() public pure {
		assert(bytes1("") & ("") == bytes1(0)); // should hold
		assert(bytes1(0xAA) & bytes1(0x55) == bytes1(0)); // should hold
		assert(bytes1(0xFF) & bytes1(0xAA) == bytes1(0xAA)); // should hold
		assert(bytes1(0xFF) & bytes1(0xAA) == bytes1(0)); // should fail
	}
}
// ----
// Warning 6328: (269-317): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
