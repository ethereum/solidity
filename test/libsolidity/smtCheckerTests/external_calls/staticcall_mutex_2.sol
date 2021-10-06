contract C {
	uint x;

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

	function f(address _a) public {
		uint y = x;
		_a.staticcall("aaaaa");
		assert(y == x); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Warning 9302: (212-234): Return value of low-level calls not used.
// Warning 2018: (164-271): Function state mutability can be restricted to view
// Info 1180: Reentrancy property(ies) for :C:\n(<errorCode> <= 0)\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(y == x)\n
