contract C  {
	function f(int x, int y) public pure returns (int) {
		return x / y;
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 4984: (77-82='x / y'): CHC: Overflow (resulting value larger than 0x80 * 2**248 - 1) happens here.
// Warning 4281: (77-82='x / y'): CHC: Division by zero happens here.
