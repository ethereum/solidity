pragma experimental SMTChecker;

contract C
{
	function f() internal pure returns (uint, uint) {
		return (2, 3);
	}
	function g() public pure {
		uint x;
		uint y;
		(x,y) = f();
		assert(x == 1);
		assert(y == 4);
	}
}
// ----
// Warning 6328: (182-196): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\ng()
// Warning 6328: (200-214): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\ng()
