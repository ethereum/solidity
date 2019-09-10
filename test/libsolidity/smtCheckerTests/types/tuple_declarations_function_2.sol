pragma experimental SMTChecker;

contract C
{
	function f(uint x) internal pure returns (uint, bool, uint) {
		bool b = true;
		uint y = 999;
		return (x * 2, b, y);
	}
	function g() public pure {
		(uint x, bool b, uint y) = f(7);
		assert(x == 14);
		assert(b);
		assert(y == 999);
	}
}
// ----
// Warning: (152-157): Overflow (resulting value larger than 2**256 - 1) happens here
