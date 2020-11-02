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
		// Removed because Spacer 4.8.9 seg faults.
		//d.d();
		assert(oldX == x);
	}
}
// ----
// Warning 2018: (236-355): Function state mutability can be restricted to view
