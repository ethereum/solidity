pragma experimental SMTChecker;

contract C {
	uint x = 5;

	constructor() {
		assert(x == 5);
		x = 10;
	}

	function f(uint y) public view {
		assert(y == x);
	}
}
// ----
// Warning 6328: (145-159): CHC: Assertion violation happens here.\nCounterexample:\nx = 10\ny = 11\n\nTransaction trace:\nC.constructor()\nState: x = 10\nC.f(11)
