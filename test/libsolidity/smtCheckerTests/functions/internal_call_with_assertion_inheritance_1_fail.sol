pragma experimental SMTChecker;

contract A {
	uint x;
	function f() internal {
		assert(x == 2);
		--x;
	}
}

contract C is A {
	constructor() {
		assert(x == 1);
		++x;
		f();
		assert(x == 1);
	}
}
// ----
// Warning 6328: (82-96): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n\n\n\nTransaction trace:\nconstructor()
// Warning 6328: (148-162): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()
// Warning 6328: (180-194): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()
