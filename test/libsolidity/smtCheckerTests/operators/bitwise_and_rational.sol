contract C {
	function f() public pure {
		assert(1 & 0 != 0);
		assert(-1 & 3 == 3);
		assert(-1 & -1 == -1);
		assert(-1 & 127 == 127);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (43-61): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
