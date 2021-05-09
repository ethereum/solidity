uint constant A = 42;
contract C {
	function f(uint x) public pure returns (uint) {
		return x + A;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 4984: (93-98): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\nx = 115792089237316195423570985008687907853269984665640564039457584007913129639894\n = 0\n\nTransaction trace:\nC.constructor()\nC.f(115792089237316195423570985008687907853269984665640564039457584007913129639894)
