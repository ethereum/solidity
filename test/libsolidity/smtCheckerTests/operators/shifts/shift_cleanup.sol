pragma experimental SMTChecker;

contract C {
	function f() public pure returns (uint16 x) {
		x = 0xffff;
		x += 32;
		x = x << 8;
		x = x >> 16;
		assert(x == 0);
		// Fails because x = 0.
		assert(x == 10);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 4984: (109-116): CHC: Overflow (resulting value larger than 65535) happens here.
// Warning 6328: (193-208): CHC: Assertion violation happens here.
