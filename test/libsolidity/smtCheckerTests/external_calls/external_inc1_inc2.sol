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
		assert(oldX == x); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (253-270): CHC: Assertion violation happens here.
