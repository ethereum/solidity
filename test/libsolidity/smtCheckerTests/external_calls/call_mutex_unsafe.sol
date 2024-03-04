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
// SMTIgnoreCex: no
// ----
// Warning 9302: (212-228): Return value of low-level calls not used.
// Warning 6328: (232-246): CHC: Assertion violation happens here.\nCounterexample:\nx = 0, lock = false\n_a = 0x0\ny = 1\n\nTransaction trace:\nC.constructor()\nState: x = 0, lock = false\nC.set(1)\nState: x = 1, lock = false\nC.f(0x0)\n    _a.call("aaaaa") -- untrusted external call, synthesized as:\n        C.set(0) -- reentrant call
