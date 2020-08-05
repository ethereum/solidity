pragma experimental SMTChecker;

abstract contract D {
	function d() external virtual;
}

contract C {
	uint x;
	uint y;
	D d;

	function inc2() public {
		if (y == 1)
			x = 1;
	}
	function inc1() public {
		if (x == 0)
			y = 1;
	}

	function f() public {
		uint oldX = x;
		d.d();
		assert(oldX == x);
	}
}
// ----
// Warning 6328: (286-303): Assertion violation happens here
