contract C
{
	function f() internal pure returns (uint, bool, uint) {
		uint x = 3;
		bool b = true;
		uint y = 999;
		return (x, b, y);
	}
	function g() public pure {
		(, bool b,) = f();
		assert(!b);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (191-201): CHC: Assertion violation happens here.\nCounterexample:\n\nb = true\n\nTransaction trace:\nC.constructor()\nC.g()\n    C.f() -- internal call
