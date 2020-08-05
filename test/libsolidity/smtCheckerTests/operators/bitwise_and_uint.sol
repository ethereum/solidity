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
// Warning 6328: (107-125): Assertion violation happens here
// Warning 6328: (180-203): Assertion violation happens here
// Warning 6328: (207-230): Assertion violation happens here
