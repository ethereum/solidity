pragma experimental SMTChecker;

contract C {
	function f() public pure {
		bytes32 x = 0x00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff;
		bytes1 z = 0x00;
		bytes1 o = 0xff;
		assert(x[0] == z);
		assert(x[31] == o);
		assert(x[0] == x[22]);
		assert(x[0] == x[23]);
	}
}
// ----
// Warning 6328: (264-285): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
