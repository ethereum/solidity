pragma experimental SMTChecker;

contract C {
	function f() public pure {
		uint8 x = 1;
		uint16 y = 0;
		assert(x | y != 0);
		x = 0xff;
		y = 0xff00;
		assert(x | y == 0xff);
		assert(x | y == 0xffff);
		assert(x | y == 0x0000);
	}
}
// ----
// Warning 6328: (155-176): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 255\ny = 65280\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (207-230): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 255\ny = 65280\n\nTransaction trace:\nC.constructor()\nC.f()
