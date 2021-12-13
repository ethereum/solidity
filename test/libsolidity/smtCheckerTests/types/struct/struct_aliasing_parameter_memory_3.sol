contract C {
    struct S {
		uint sum;
		uint[] a;
    }

	struct T {
		S s;
		uint x;
	}

	function f(T memory m, uint v) internal pure {
		m.s.sum = v;
		m.s.a = new uint[](2);
	}

	constructor(uint amt) {
		T memory t;
		f(t, amt);
		assert(t.s.a.length == 2); // should hold but no aliasing support means it fails for now
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (238-263): CHC: Assertion violation happens here.\nCounterexample:\n\namt = 0\nt = {s: {sum: 0, a: []}, x: 0}\n\nTransaction trace:\nC.constructor(0)
