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
		// Fails due to the if.
		assert(x >= 17);
	}
}
// ----
// Warning: (223-238): Assertion violation happens here
