pragma experimental SMTChecker;

contract C
{
	function f(uint x) public pure {
		require(x < 10000);
		uint y = x * 2;
		assert((y % 2) == 0);
	}
}
