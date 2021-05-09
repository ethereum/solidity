contract A {
	uint x;
	function f() internal view {
		assert(x == 0);
	}
}

contract B is A {
	uint a;
	uint b;
}

contract C is B {
	uint y;
	uint z;
	uint w;
	function g() public {
		x = 1;
		f();
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (54-68): CHC: Assertion violation happens here.\nCounterexample:\ny = 0, z = 0, w = 0, a = 0, b = 0, x = 1\n\nTransaction trace:\nC.constructor()\nState: y = 0, z = 0, w = 0, a = 0, b = 0, x = 0\nC.g()\n    A.f() -- internal call
