pragma experimental SMTChecker;

contract A {
	uint public x;
	constructor(uint) {}

	function f() internal returns (uint) {
		x = x + 1;
		return x;
	}
}

contract C is A {
	constructor() A(f()) {
		assert(x == 1);
		assert(x == 0); // should fail
		assert(x > 2000); // should fail
	}
}
// ----
// Warning 6328: (218-232): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n\n\n\nTransaction trace:\nconstructor()
// Warning 6328: (251-267): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n\n\n\nTransaction trace:\nconstructor()
