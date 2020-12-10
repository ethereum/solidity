pragma experimental SMTChecker;

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
// ----
// Warning 6328: (224-234): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\ng()
