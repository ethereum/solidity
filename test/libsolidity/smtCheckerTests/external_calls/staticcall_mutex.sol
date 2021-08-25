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
