pragma experimental SMTChecker;

contract C {
	function f() public pure {
		uint8 x = 0xff;
		assert(~x == 0x00);
		uint16 y = 0xff00;
		assert(~y == 0xff);
		assert(~y == 0xffff);
		assert(~y == 0x0000);
	}
}
// ----
// Warning 6328: (159-179): Assertion violation happens here
// Warning 6328: (183-203): Assertion violation happens here
