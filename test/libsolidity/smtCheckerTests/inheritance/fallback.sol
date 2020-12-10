pragma experimental SMTChecker;

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
// ----
// Warning 6328: (122-136): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nfallback()
// Warning 6328: (171-185): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\ng()
// Warning 6328: (288-302): CHC: Assertion violation happens here.\nCounterexample:\ny = 0, x = 0\n\n\n\nTransaction trace:\nconstructor()\nState: y = 0, x = 0\nfallback()
