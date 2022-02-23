contract C {
	uint[] public a;
	constructor() {
		a.push();
		a.push();
		a.push();
		a.push();
	}
	function f() public view {
		uint y = this.a(2);
		assert(y == a[2]); // should hold
		assert(y == 1); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (187-201): CHC: Assertion violation happens here.\nCounterexample:\na = [0, 0, 0, 0]\ny = 0\n\nTransaction trace:\nC.constructor()\nState: a = [0, 0, 0, 0]\nC.f()
// Info 1180: Contract invariant(s) for :C:\n!(a.length <= 2)\n
