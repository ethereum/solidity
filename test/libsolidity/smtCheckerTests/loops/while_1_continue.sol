pragma experimental SMTChecker;

contract C
{
	function f(uint x, bool b) public pure {
		require(x < 100);
		while (x < 10) {
			if (b) {
				x = 15;
				continue;
			}
			else
				x = 20;

		}
		// Should be safe, but fails due to continue being unsupported
		// and erasing all knowledge.
		assert(x >= 15);
	}
}
// ----
// Warning: (294-309): Assertion violation happens here
