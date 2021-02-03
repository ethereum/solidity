pragma experimental SMTChecker;

contract B {
	uint x;
	function f() public view {
		assert(x == 0);
	}
}

contract C is B {
	uint y;
	function g() public {
		x = 1;
		f();
	}
}
// ----
// Warning 6328: (85-99): CHC: Assertion violation happens here.\nCounterexample:\ny = 0, x = 1\n\nTransaction trace:\nC.constructor()\nState: y = 0, x = 0\nC.g()\n    B.f() -- internal call\nState: y = 0, x = 1\nB.f()
