interface I {
	function ext() external;
}

contract C {
	bool lock;
	modifier mutex {
		require(!lock);
		lock = true;
		_;
		lock = false;
	}
	function f(I _i) public mutex {
		uint x = address(this).balance;
		_i.ext();
		assert(address(this).balance == x); // should hold
		assert(address(this).balance < x); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (277-310): CHC: Assertion violation happens here.\nCounterexample:\nlock = true\n_i = 0\nx = 730\n\nTransaction trace:\nC.constructor()\nState: lock = false\nC.f(0)\n    _i.ext() -- untrusted external call
