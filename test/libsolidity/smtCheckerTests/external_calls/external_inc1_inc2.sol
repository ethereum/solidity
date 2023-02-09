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
// ====
// SMTEngine: all
// ----
// Warning 2018: (203-322): Function state mutability can be restricted to view
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
