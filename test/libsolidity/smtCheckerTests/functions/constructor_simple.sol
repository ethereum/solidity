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
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6328: (141-155): CHC: Assertion violation happens here.
