pragma experimental SMTChecker;

contract C {
	function f() public pure {
		int16 x = 1;
		assert(~x == 0);
		x = 0xff;
		assert(~x == 0);
		x = 0x0f;
		assert(~x == 0xf0);
		x = -1;
		assert(~x != 0);
		x = -2;
		assert(~x == 1);
	}
}
// ----
// Warning 6328: (91-106): Assertion violation happens here
// Warning 6328: (122-137): Assertion violation happens here
// Warning 6328: (153-171): Assertion violation happens here
// Warning 6328: (185-200): Assertion violation happens here
