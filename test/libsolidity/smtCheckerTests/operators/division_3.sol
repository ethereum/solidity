pragma experimental SMTChecker;
contract C {
	function f(int x, int y) public pure returns (int) {
		require(y != 0);
		return x / y;
	}
}
// ----
// Warning 4984: (127-132): CHC: Overflow (resulting value larger than 0x80 * 2**248 - 1) happens here.\nCounterexample:\n\nx = (- 57896044618658097711785492504343953926634992332820282019728792003956564819968)\ny = (- 1)\n = 0\n\nTransaction trace:\nconstructor()\nf((- 57896044618658097711785492504343953926634992332820282019728792003956564819968), (- 1))
