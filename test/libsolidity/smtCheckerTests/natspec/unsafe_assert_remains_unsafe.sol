contract C {
	uint x;
	uint y;

	function g(uint _x) public {
		f1(_x);
		assert(x > 0); // should fail

		f2(_x);
		assert(y > 0); // should fail
	}

	/// @custom:smtchecker abstract-function-nondet
	function f1(uint _x) internal {
		x = _x;
	}

	function f2(uint _y) internal {
		y = _y;
	}

}
// ====
// SMTEngine: chc
// ----
// Warning 6328: (74-87): CHC: Assertion violation happens here.\nCounterexample:\nx = 0, y = 0\n_x = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0, y = 0\nC.g(0)\n    C.f1(0) -- internal call
// Warning 6328: (117-130): CHC: Assertion violation happens here.\nCounterexample:\nx = 1, y = 0\n_x = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0, y = 0\nC.g(0)\n    C.f1(0) -- internal call\n    C.f2(0) -- internal call
