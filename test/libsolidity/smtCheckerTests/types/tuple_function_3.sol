pragma experimental SMTChecker;

contract C
{
	function f() internal pure returns (uint, bool, uint) {
		return (2, false, 3);
	}
	function g() public pure {
		uint x;
		uint y;
		bool b;
		(,b,) = f();
		assert(x == 2);
		assert(y == 4);
		assert(!b);
	}
}
// ----
// Warning 6328: (205-219): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\ny = 0\nb = false\n\nTransaction trace:\nC.constructor()\nC.g()\n    C.f() -- internal call
// Warning 6328: (223-237): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\ny = 0\nb = false\n\nTransaction trace:\nC.constructor()\nC.g()\n    C.f() -- internal call
