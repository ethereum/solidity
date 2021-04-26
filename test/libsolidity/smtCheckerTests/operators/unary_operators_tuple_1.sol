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
