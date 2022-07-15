contract C  {
	function f(int x, int y) public pure returns (int) {
		return x - y;
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 3944: (77-82): CHC: Underflow (resulting value less than -2**255) happens here.
// Warning 4984: (77-82): CHC: Overflow (resulting value larger than 2**255 - 1) happens here.
