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
		// Disabled because of Spacer nondeterminism.
		//assert(y == x);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 2072: (251-257): Unused local variable.
