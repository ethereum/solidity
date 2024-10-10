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
// SMTEngine: chc
// SMTIgnoreCex: yes
// ----
// Warning 6328: (274-288): CHC: Assertion violation happens here.\nCounterexample:\nx = 0, d = 0, lock = false\ny = 1\n\nTransaction trace:\nC.constructor()\nState: x = 0, d = 0, lock = false\nC.set(1)\nState: x = 1, d = 0, lock = false\nC.f()\n    d.d() -- untrusted external call, synthesized as:\n        C.set(0) -- reentrant call
