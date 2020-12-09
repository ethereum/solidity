pragma experimental SMTChecker;

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
// ----
// Warning 6328: (307-321): CHC: Assertion violation happens here.\nCounterexample:\nx = 1, d = 0, lock = false\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0, d = 0, lock = false\nf()
