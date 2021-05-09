contract C {
	uint[] a;
	function f() public {
		a.length;
		a.pop();
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2529: (61-68): CHC: Empty array "pop" happens here.\nCounterexample:\na = []\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.f()
