pragma experimental SMTChecker;

contract C {
	mapping (uint => uint[]) public m;

	constructor() {
		m[0].push();
		m[0].push();
		m[0][1] = 42;
	}

	function f() public view {
		uint y = this.m(0,1);
		assert(y == m[0][1]); // should hold
		assert(y == 1); // should fail
	}
}
// ----
// Warning 6328: (243-257): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
