library L {
	function l(uint x, uint y) internal pure returns (uint) {
		return x + y;
	}
}

contract C {
	function f(uint u, uint s, bool b) internal pure returns (uint z) {
		if (b)
			z = u;
		else
			z = s;
	}

	using L for uint;

	function call() public pure {
		uint a = 2;
		uint b = a.l({y: 3});
		assert(b == 5);
		b = L.l({x: 3, y: 3});
		assert(b == 6);
		b = f({b: true, u: 1, s: 2});
		assert(b == 1);
		b = f({b: false, u: 1, s: 2});
		// Fails, should be 2.
		assert(b == 6);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (475-489): CHC: Assertion violation happens here.\nCounterexample:\n\na = 2\nb = 2\n\nTransaction trace:\nC.constructor()\nC.call()\n    L.l(2, 3) -- internal call\n    L.l(3, 3) -- internal call\n    C.f(1, 2, true) -- internal call\n    C.f(1, 2, false) -- internal call
