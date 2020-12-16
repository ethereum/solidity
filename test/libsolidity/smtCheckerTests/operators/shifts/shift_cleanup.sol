pragma experimental SMTChecker;

contract C {
	function f() public pure returns (uint16 x) {
		unchecked {
			x = 0xffff;
			x += 32;
			x = x << 8;
			x = x >> 16;
		}
		assert(x == 0);
		// Fails because x = 0.
		assert(x == 10);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6328: (215-230): CHC: Assertion violation happens here.
