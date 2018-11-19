pragma experimental SMTChecker;

library L
{
	function add(uint x, uint y) internal pure returns (uint) {
		require(x < 1000);
		require(y < 1000);
		return x + y;
	}
}

contract C
{
	using L for uint;
	function f(uint x) public pure {
		uint y = x.add(999);
		assert(y < 10000);
	}
}
