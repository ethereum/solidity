contract B {
	uint x = 5;
}

contract C is B {
	constructor() {
		assert(x == 5);
		x = 10;
	}

	function f(uint y) public view {
		assert(y == x);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (132-146): CHC: Assertion violation happens here.
