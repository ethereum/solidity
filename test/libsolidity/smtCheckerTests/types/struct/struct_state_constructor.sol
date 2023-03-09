contract C {

	struct S {
		uint x;
	}

	S s = S(42);

	function test() view public {
		assert(s.x == 42);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
