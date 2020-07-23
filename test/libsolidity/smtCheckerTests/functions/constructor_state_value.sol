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
// Warning 4661: (145-159): Assertion violation happens here
