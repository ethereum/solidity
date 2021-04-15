contract C {
	uint[] a;
	function f() public {
		a.push();
		assert(a[a.length - 1] == 100);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (61-91): CHC: Assertion violation happens here.\nCounterexample:\na = [0]\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.f()
