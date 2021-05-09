contract C {
	uint[] a;
	function g() internal view {
		a.length;
	}
	function f() public {
		a.pop();
		g();
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2529: (94-101): CHC: Empty array "pop" happens here.\nCounterexample:\na = []\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.f()
