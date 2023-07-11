contract C {
	function f(bool b) public pure {
		uint x;
		if (b) ++(x);
		if (b) --(x);
		if (b) delete(b);
		assert(x == 0);
		assert(!b);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
