contract C {
	function f(uint a) public pure {
		assert(a <= type(uint).max);
		assert(a >= type(uint).min);
		require(a <= type(uint64).max);
		assert(a <= type(uint64).max);
		assert(a <= type(uint32).max); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (178-207): CHC: Assertion violation happens here.\nCounterexample:\n\na = 4294967296\n\nTransaction trace:\nC.constructor()\nC.f(4294967296)
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
