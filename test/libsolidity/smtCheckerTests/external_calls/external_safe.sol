pragma experimental SMTChecker;

abstract contract D {
	function d() external virtual;
}

contract C {
	uint x;
	D d;
	function f() public {
		if (x < 10)
			++x;
	}
	function g() public {
		d.d();
		assert(x < 11);
	}
}
// ----
// Warning 6328: (200-214): CHC: Assertion violation might happen here.
// Warning 4661: (200-214): BMC: Assertion violation happens here.
