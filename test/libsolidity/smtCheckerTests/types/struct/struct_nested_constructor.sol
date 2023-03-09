contract C {

	struct S {
		uint x;
	}

	struct T {
		S s;
		uint y;
	}

	function test() pure public {
		S memory inner = S(43);
		T memory outer = T(inner, 512);
		assert(outer.y == 512);
		assert(outer.s.x == 43);
		assert(outer.s.x == 42);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (219-242): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
