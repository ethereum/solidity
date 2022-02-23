contract C {
	uint x;
	uint y;

	function g(uint _x) public {
		f1(_x);
		// If the body of function `f` is ignored while keeping the state,
		// the assertion is true and not reporting it would be a false negative.
		// However, since `f` can change the state, the state variables are also
		// assigned nondeterministic values after a call to `f`.
		// Therefore the assertion below should fail.
		assert(x == 0);

		f2(_x);
		assert(y == 0); // should fail
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
// Warning 6328: (400-414): CHC: Assertion violation happens here.\nCounterexample:\nx = 1, y = 0\n_x = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0, y = 0\nC.g(0)\n    C.f1(0) -- internal call
// Warning 6328: (429-443): CHC: Assertion violation happens here.\nCounterexample:\nx = 1, y = 1\n_x = 1\n\nTransaction trace:\nC.constructor()\nState: x = 0, y = 0\nC.g(1)\n    C.f1(1) -- internal call\n    C.f2(1) -- internal call
