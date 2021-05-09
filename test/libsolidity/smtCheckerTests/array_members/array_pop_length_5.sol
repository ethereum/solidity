contract C {
	uint[] a;
	function g() internal {
		a.push();
	}
	function f() public {
		a.pop();
		g();
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2529: (89-96): CHC: Empty array "pop" happens here.\nCounterexample:\na = []\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.f()
