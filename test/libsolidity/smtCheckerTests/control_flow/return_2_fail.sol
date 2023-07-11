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
// Warning 6328: (270-296): CHC: Assertion violation happens here.
// Warning 6328: (300-317): CHC: Assertion violation happens here.
// Warning 6328: (321-347): CHC: Assertion violation happens here.
// Warning 6328: (351-370): CHC: Assertion violation happens here.
// Warning 6328: (374-400): CHC: Assertion violation happens here.
// Warning 6328: (404-425): CHC: Assertion violation happens here.
// Warning 6328: (429-457): CHC: Assertion violation happens here.
// Warning 6328: (461-484): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
