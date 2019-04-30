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
// Warning: (182-196): Assertion violation happens here
