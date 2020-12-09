pragma experimental SMTChecker;

contract C {
	function f() public pure {
		uint8 x = 1;
		uint16 y = 0;
		assert(x & y != 0);
		x = 0xff;
		y = 0xffff;
		assert(x & y == 0xff);
		assert(x & y == 0xffff);
		assert(x & y == 0x0000);
	}
}
// ----
// Warning 6328: (107-125): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (180-203): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (207-230): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
