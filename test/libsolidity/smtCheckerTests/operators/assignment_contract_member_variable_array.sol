contract A {
	uint[] a;
	function f() public {
		A.a.push(2);
		assert(A.a[A.a.length - 1] == 2);
		A.a.pop();
		// Fails
		assert(A.a.length > 0);
		assert(A.a.length == 0);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (124-146): CHC: Assertion violation happens here.\nCounterexample:\na = []\n\nTransaction trace:\nA.constructor()\nState: a = []\nA.f()
