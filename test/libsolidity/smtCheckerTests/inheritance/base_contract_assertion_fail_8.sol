pragma experimental SMTChecker;

abstract contract A {
	uint x;
	function f() public view {
		assert(x == 2);
	}
}

contract C is  A {
	function g() public {
		x = 2;
		f();
		x = 0;
	}
}
// ----
// Warning 6328: (94-108): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0\nA.f()
