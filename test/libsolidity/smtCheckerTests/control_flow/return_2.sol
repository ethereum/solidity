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
		assert(add(100, 0) == 100);
		assert(c == 0xff);
		assert(add(100, 1) == 101);
		assert(c == 0xffff);
		assert(add(100, 2) == 102);
		assert(c == 0xffffff);
		assert(add(100, 100) == 200);
		assert(c == 0xffffffff);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 11 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
