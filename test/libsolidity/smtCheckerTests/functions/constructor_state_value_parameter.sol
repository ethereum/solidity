contract C {
	uint x = 5;

	constructor(uint a, uint b) {
		assert(x == 5);
		x = a + b;
	}

	function f(uint y) view public {
		assert(y == x);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (129-143): CHC: Assertion violation happens here.
// Warning 4984: (82-87): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
