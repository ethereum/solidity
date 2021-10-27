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

	function f(address _a) mutex public {
		uint y = x;
		_a.staticcall("aaaaa");
		assert(y == x); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Warning 9302: (218-240): Return value of low-level calls not used.
// Info 1180: Reentrancy property(ies) for :C:\n((!lock || (<errorCode> <= 0)) && (lock' || !lock))\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(y == x)\n
