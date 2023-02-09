contract C {
	function f(bool b) public pure {
		uint x;
		if (b) ++(x);
		else x += 1;
		assert(x == 1);
		assert(!b);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (108-118): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
