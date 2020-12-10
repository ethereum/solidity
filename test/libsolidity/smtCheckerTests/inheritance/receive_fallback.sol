pragma experimental SMTChecker;

// 2 warnings, receive and A.g
contract A {
	uint x;

	receive () external payable {
		assert(x == 1);
	}
	function g() public view {
		assert(x == 1);
	}
}

// 3 warnings, fallback, A.receive and A.g
contract B is A {
	uint y;

	fallback () external {
		assert(x == 1);
	}
}
// ----
// Warning 6328: (120-134): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nreceive()
// Warning 6328: (169-183): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\ng()
// Warning 6328: (288-302): CHC: Assertion violation happens here.\nCounterexample:\ny = 0, x = 0\n\n\n\nTransaction trace:\nconstructor()\nState: y = 0, x = 0\nfallback()
