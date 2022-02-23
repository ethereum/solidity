contract C  {
	function f(int x, int y) public pure returns (int) {
		return x * y;
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 3944: (77-82): CHC: Underflow (resulting value less than -57896044618658097711785492504343953926634992332820282019728792003956564819968) happens here.
// Warning 4984: (77-82): CHC: Overflow (resulting value larger than 0x80 * 2**248 - 1) happens here.
