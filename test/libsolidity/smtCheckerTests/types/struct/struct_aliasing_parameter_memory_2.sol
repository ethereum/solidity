contract C {
    struct S {
		uint sum;
		uint[] a;
    }

	struct T {
		S s;
		uint x;
	}

	function f(S memory m, uint v) internal pure {
		m.sum = v;
		m.a = new uint[](2);
	}

	constructor(uint amt) {
		T memory t;
		f(t.s, amt);
		assert(t.s.a.length == 2); // should hold but no aliasing support means it fails for now
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (236-261): CHC: Assertion violation happens here.\nCounterexample:\n\namt = 0\nt = {s: {sum: 0, a: []}, x: 0}\n\nTransaction trace:\nC.constructor(0)
