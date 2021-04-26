contract C {
	uint x = f(2);

	function f(uint y) internal pure returns (uint) {
		assert(y > 1000);
		return y;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (83-99): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nC.constructor()
