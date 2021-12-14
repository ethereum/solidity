contract C {
    struct S {
		uint sum;
		uint[] a;
    }

	function f(S memory m, uint v) internal pure {
		m.sum = v;
		m.a = new uint[](2);
	}

	constructor(uint amt) {
		S memory s;
		f(s, amt);
		assert(s.a.length == 2); // should hold but no aliasing support means it fails for now
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (201-224): CHC: Assertion violation happens here.\nCounterexample:\n\namt = 0\ns = {sum: 0, a: []}\n\nTransaction trace:\nC.constructor(0)
