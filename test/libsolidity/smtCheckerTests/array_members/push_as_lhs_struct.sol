contract C {
	struct S {
		int[] b;
	}
	S s;
	struct T {
		S s;
	}
	T t;
	function f() public {
		s.b.push() = t.s.b.push();
		assert(s.b[s.b.length -1] == t.s.b[t.s.b.length - 1]);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 5 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
