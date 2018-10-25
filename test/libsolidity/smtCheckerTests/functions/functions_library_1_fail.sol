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
	function f(uint x) public pure {
		uint y = L.add(x, 999);
		assert(y < 1000);
	}
}
// ----
// Warning: (245-261): Assertion violation happens here
