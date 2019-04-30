pragma experimental SMTChecker;

contract C
{
	function f(uint16 x, uint16 y) public pure {
		require(y > 0);
		uint z = x % y;
		assert(z < 100_000);
	}
}
