contract C  {
	function f(int x, int y) public pure returns (int) {
		require(x + y >= x);
		return x + y;
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 3944: (78-83): CHC: Underflow (resulting value less than -2**255) happens here.
// Warning 4984: (78-83): CHC: Overflow (resulting value larger than 2**255 - 1) happens here.
