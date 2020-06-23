pragma experimental SMTChecker;

contract C {
	uint x;

	constructor() {
		assert(x == 0);
		x = 10;
	}

	function f(uint y) public view {
		assert(y == x);
	}
}
// ----
// Warning 4661: (141-155): Assertion violation happens here
