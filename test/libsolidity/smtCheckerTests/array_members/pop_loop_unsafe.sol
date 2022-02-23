contract C {
	uint[] a;
	function f(uint l) public {
		for (uint i = 0; i < l; ++i) {
			a.push();
			a.pop();
		}
		a.pop();
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2529: (117-124): CHC: Empty array "pop" happens here.\nCounterexample:\na = []\nl = 0\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.f(0)
