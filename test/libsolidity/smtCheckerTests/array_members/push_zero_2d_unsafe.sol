contract C {
	uint[][] a;
	function f() public {
		a.push();
		a[a.length - 1].push();
		assert(a[a.length - 1][0] == 100);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (89-122): CHC: Assertion violation happens here.\nCounterexample:\na = [[0]]\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.f()
