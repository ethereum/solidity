contract C {

	struct S {
		uint x;
	}

	function test() pure public {
		assert(S(42).x == 42);
	}
}
// ====
// SMTEngine: all
// ----
