// 2 warnings, receive and A.g
contract A {
	uint x;

	receive () external virtual payable {
		assert(x == 1);
	}
	function g() public view {
		assert(x == 1);
	}
}

// 2 warnings, receive and A.g
contract B is A {
	uint y;

	receive () external payable override {
		assert(x == 1);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (95-109): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nA.constructor()\nState: x = 0\nA.receive()
// Warning 6328: (144-158): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nA.constructor()\nState: x = 0\nA.g()
// Warning 6328: (267-281): CHC: Assertion violation happens here.\nCounterexample:\ny = 0, x = 0\n\nTransaction trace:\nB.constructor()\nState: y = 0, x = 0\nB.receive()
