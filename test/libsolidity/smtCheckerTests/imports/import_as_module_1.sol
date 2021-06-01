==== Source: A ====
import "s1.sol" as M;
contract D is M.C {
	function f(uint _y) public {
		g(_y);
		assert(x == _y); // should hold
		assert(x > 100); // should fail
	}
}
==== Source: s1.sol ====
contract C {
	uint x;
	function g(uint _x) public {
		x = _x;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (A:117-132): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n_y = 0\n\nTransaction trace:\nD.constructor()\nState: x = 0\nD.f(0)\n    C.g(0) -- internal call
