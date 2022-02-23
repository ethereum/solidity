// 2 warnings, fallback and A.g
contract A {
	uint x;

	fallback () external virtual {
		assert(x == 1);
	}
	function g() public view {
		assert(x == 1);
	}
}

// 2 warnings, fallback and A.g
contract B is A {
	uint y;

	fallback () external override {
		assert(x == 1);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (89-103): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nA.constructor()\nState: x = 0\nA.fallback()
// Warning 6328: (138-152): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nA.constructor()\nState: x = 0\nA.g()
// Warning 6328: (255-269): CHC: Assertion violation happens here.\nCounterexample:\ny = 0, x = 0\n\nTransaction trace:\nB.constructor()\nState: y = 0, x = 0\nB.fallback()
