pragma experimental SMTChecker;

contract C
{
	function f(uint x, bool b) public pure {
		require(x < 100);
		while (x < 10) {
			if (b)
				x = x + 1;
			else
				x = 0;
		}
		assert(x > 0);
	}
}
// ----
// Warning: (177-190): Assertion violation happens here
