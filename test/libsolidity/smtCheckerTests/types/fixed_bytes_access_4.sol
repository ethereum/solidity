pragma experimental SMTChecker;

contract C {
	function f() public pure {
		bytes32 x = 0x00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff;
		byte z = 0x00;
		byte o = 0xff;
		assert(x[0] == z);
		assert(x[31] == o);
		assert(x[0] == x[22]);
		assert(x[0] == x[23]);
	}
}
// ----
// Warning 6328: (260-281): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
