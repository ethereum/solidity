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
		_a.call("aaaaa");
		assert(y == x); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 9302: (212-228='_a.call("aaaaa")'): Return value of low-level calls not used.
// Warning 6328: (232-246='assert(y == x)'): CHC: Assertion violation happens here.
