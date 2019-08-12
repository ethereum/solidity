pragma experimental SMTChecker;

contract C
{
	function f(uint x) public pure returns (uint, uint) {
		return (x, x);
	}

	function g() public pure {
		(uint a, uint b) = f(0);
		(uint c, uint d) = f(0);
		assert(a == c && b == d);
	}
}
