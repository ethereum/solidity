pragma experimental SMTChecker;

contract C  {
	function f(int x, int y) public pure returns (int) {
		require(x + y >= x);
		return x + y;
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 3944: (111-116): CHC: Underflow (resulting value less than -57896044618658097711785492504343953926634992332820282019728792003956564819968) happens here.
// Warning 4984: (111-116): CHC: Overflow (resulting value larger than 0x80 * 2**248 - 1) happens here.
