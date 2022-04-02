contract C {
	uint c;
	function add(uint x, uint y) internal returns (uint) {
		c = 0xff;
		if (y == 0)
			return x;
		c = 0xffff;
		if (y == 1)
			return ++x;
		c = 0xffffff;
		if (y == 2)
			return x + 2;
		c = 0xffffffff;
		return x + y;
	}

	function f() public {
		assert(add(100, 0) != 100);
		assert(c != 0xff);
		assert(add(100, 1) != 101);
		assert(c != 0xffff);
		assert(add(100, 2) != 102);
		assert(c != 0xffffff);
		assert(add(100, 100) != 200);
		assert(c != 0xffffffff);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (270-296='assert(add(100, 0) != 100)'): CHC: Assertion violation happens here.
// Warning 6328: (300-317='assert(c != 0xff)'): CHC: Assertion violation happens here.
// Warning 6328: (321-347='assert(add(100, 1) != 101)'): CHC: Assertion violation happens here.
// Warning 6328: (351-370='assert(c != 0xffff)'): CHC: Assertion violation happens here.
// Warning 6328: (374-400='assert(add(100, 2) != 102)'): CHC: Assertion violation happens here.
// Warning 6328: (404-425='assert(c != 0xffffff)'): CHC: Assertion violation happens here.
// Warning 6328: (429-457='assert(add(100, 100) != 200)'): CHC: Assertion violation happens here.
// Warning 6328: (461-484='assert(c != 0xffffffff)'): CHC: Assertion violation happens here.
