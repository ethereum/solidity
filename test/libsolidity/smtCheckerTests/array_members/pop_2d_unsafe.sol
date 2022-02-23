contract C {
	uint[][] a;
	function f() public {
		a.push();
		a.push();
		a[0].push();
		a[1].pop();
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2529: (90-100): CHC: Empty array "pop" happens here.\nCounterexample:\na = [[0], []]\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.f()
