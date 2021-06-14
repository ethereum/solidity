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

	function f() mutex public {
		uint y = x;
		d.d();
		assert(y == x);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 0: (57-300): Contract invariants for :C:\n((!lock || ((x' + ((- 1) * x)) = 0)) && (<errorCode> <= 0) && (lock' || !lock))\n
