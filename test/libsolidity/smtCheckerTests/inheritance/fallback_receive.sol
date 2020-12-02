pragma experimental SMTChecker;

// 2 warnings, fallback and A.g
contract A {
	uint x;

	fallback () external {
		assert(x == 1);
	}
	function g() public view {
		assert(x == 1);
	}
}

// 3 warnings, receive, A.fallback and A.g
contract B is A {
	uint y;

	receive () external payable {
		assert(x == 1);
	}
}
// ----
// Warning 6328: (114-128): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nfallback()
// Warning 6328: (163-177): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\ng()
// Warning 6328: (289-303): CHC: Assertion violation happens here.\nCounterexample:\ny = 0, x = 0\n\n\n\nTransaction trace:\nconstructor()\nState: y = 0, x = 0\nreceive()
