pragma experimental SMTChecker;

contract C  {
	function f(uint x, uint y) public pure returns (uint) {
		return x * y;
	}
}
// ----
// Warning 4984: (113-118): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\nx = 57896044618658097711785492504343953926634992332820282019728792003956564819968\ny = 2\n = 0\n\nTransaction trace:\nconstructor()\nf(57896044618658097711785492504343953926634992332820282019728792003956564819968, 2)
