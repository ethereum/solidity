pragma experimental SMTChecker;

contract C
{
	function f(uint x) public pure {
		require(x < 100);
		while (x < 10) {
			x = x + 1;
		}
		assert(x < 14);
	}
}
// ----
// Warning: (139-153): Assertion violation happens here
