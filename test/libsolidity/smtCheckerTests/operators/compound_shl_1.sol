contract C {
	function f(bool b) public pure {
		uint v = 1000000;
		if (b)
			v <<= 2;
		assert(v > 0);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
