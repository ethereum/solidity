pragma experimental SMTChecker;

abstract contract D {
	function d() external virtual;
}

contract C {
	uint x;
	D d;
	function f() public {
		if (x < 10)
			++x;
	}
	function g() public {
		d.d();
		assert(x < 10);
	}
}
// ----
// Warning 6328: (200-214): CHC: Assertion violation happens here.\nCounterexample:\nx = 10, d = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0, d = 0\nf()\nState: x = 1, d = 0\nf()\nState: x = 2, d = 0\ng()
