pragma experimental SMTChecker;

contract C {
	function f() public pure {
		int8 x = 1;
		int16 y = 0;
		assert(x ^ y == 1);
		int16 z = -1;
		assert(x ^ z == -2);
		assert(y ^ z == -1);
		assert(y ^ z > 0);
		x = 7; y = 3;
		assert(x ^ y < 5);
		assert(x ^ y > 5);
	}
}
// ----
// Warning 6328: (189-206): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 1\ny = 0\nz = (- 1)\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (247-264): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 7\ny = 3\nz = (- 1)\n\nTransaction trace:\nC.constructor()\nC.f()
