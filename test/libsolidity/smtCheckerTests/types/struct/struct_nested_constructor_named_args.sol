contract C {
	struct B { uint b1; }
	struct A { uint a1; B a2; }
	function f() public pure {
		A memory a = A({ a1: 1, a2: B({b1: 2}) });
		assert(a.a1 == 1 && a.a2.b1 == 2);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
