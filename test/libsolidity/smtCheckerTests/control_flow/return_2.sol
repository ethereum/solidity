pragma experimental SMTChecker;

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
// ----
