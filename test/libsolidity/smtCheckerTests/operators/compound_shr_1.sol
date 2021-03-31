contract C {
	function f(bool b) public pure {
		uint v = 1;
		if (b)
			v >>= 2;
		assert(v > 0);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (84-97): CHC: Assertion violation happens here.
