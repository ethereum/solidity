contract C  {
	function f(int x, int y) public pure returns (int) {
		return x / y;
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 4281: (77-82): CHC: Division by zero happens here.
// Warning 4984: (77-82): CHC: Overflow (resulting value larger than 0x80 * 2**248 - 1) happens here.
