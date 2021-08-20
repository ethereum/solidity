abstract contract D {
	function d() external virtual;
}

contract C {
	uint x;
	D d;

	bool lock;
	modifier mutex {
		require(!lock);
		lock = true;
		_;
		lock = false;
	}

	function set(uint _x) mutex public {
		x = _x;
	}

	function f() public {
		uint y = x;
		d.d();
		assert(y == x);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 1218: (274-288): CHC: Error trying to invoke SMT solver.
// Warning 6328: (274-288): CHC: Assertion violation might happen here.
// Warning 4661: (274-288): BMC: Assertion violation happens here.
