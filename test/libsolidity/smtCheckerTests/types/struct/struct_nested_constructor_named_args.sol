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
