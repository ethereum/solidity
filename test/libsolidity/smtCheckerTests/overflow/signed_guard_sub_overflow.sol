contract C  {
	function f(int x, int y) public pure returns (int) {
		require(x >= y);
		return x - y;
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 4984: (96-101): CHC: Overflow (resulting value larger than 0x80 * 2**248 - 1) happens here.\nCounterexample:\n\nx = 57896044618658097711785492504343953926634992332820282019728792003956564819967\ny = (- 1)\n = 0\n\nTransaction trace:\nC.constructor()\nC.f(57896044618658097711785492504343953926634992332820282019728792003956564819967, (- 1))
