pragma experimental SMTChecker;

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
// SMTIgnoreCex: yes
// ----
// Warning 6328: (275-289): CHC: Assertion violation happens here.
