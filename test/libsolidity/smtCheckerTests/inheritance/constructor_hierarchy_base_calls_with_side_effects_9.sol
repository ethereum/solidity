pragma experimental SMTChecker;

contract A {
	uint public x = 42;
	constructor(uint) {}

	function f() internal returns (uint) {
		x = x + 1;
		return x;
	}
}

contract C is A {
	constructor() A(f()) {
		assert(x == 42);
		assert(x == 0); // should fail
		assert(x == 1); // should fail
		assert(x > 2000); // should fail
	}
}
// ----
// Warning 6328: (224-238): CHC: Assertion violation happens here.\nCounterexample:\nx = 42\n\n\n\nTransaction trace:\nconstructor()
// Warning 6328: (257-271): CHC: Assertion violation happens here.\nCounterexample:\nx = 42\n\n\n\nTransaction trace:\nconstructor()
// Warning 6328: (290-306): CHC: Assertion violation happens here.\nCounterexample:\nx = 42\n\n\n\nTransaction trace:\nconstructor()
