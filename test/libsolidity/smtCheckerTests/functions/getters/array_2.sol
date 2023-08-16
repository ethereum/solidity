contract C {
	uint[][] public a;
	constructor() {
		a.push();
		a.push();
		a.push();
		a[2].push();
		a[2].push();
		a[2].push();
		a[2].push();
	}
	function f() public view {
		uint y = this.a(2,3);
		assert(y == a[2][3]); // should hold
		assert(y == 1); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: no
// ----
// Warning 6328: (242-256): CHC: Assertion violation happens here.\nCounterexample:\na = [[], [], [0, 0, 0, 0]]\ny = 0\n\nTransaction trace:\nC.constructor()\nState: a = [[], [], [0, 0, 0, 0]]\nC.f()
// Info 1391: CHC: 7 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
