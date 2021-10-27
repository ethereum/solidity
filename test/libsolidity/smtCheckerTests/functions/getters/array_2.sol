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
// SMTIgnoreCex: yes
// ----
// Warning 6328: (242-256): CHC: Assertion violation happens here.
// Info 1180: Contract invariant(s) for :C:\n!(a.length <= 2)\n!(a[2].length <= 3)\n
