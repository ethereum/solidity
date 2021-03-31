contract C {
	uint[] a;
	function f() public {
		a.pop();
		a.push();
		a.push();
		a.push();
		a.pop();
		a.pop();
		a.pop();
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2529: (49-56): CHC: Empty array "pop" happens here.\nCounterexample:\na = []\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.f()
