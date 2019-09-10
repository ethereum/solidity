pragma experimental SMTChecker;

contract C
{
	function f(uint x) public pure {
		require(x < 100);
		do {
			// Overflows due to resetting x.
			x = x + 1;
		} while (x < 10);
		assert(x < 14);
	}
}
// ----
// Warning: (179-193): Assertion violation happens here
// Warning: (150-155): Overflow (resulting value larger than 2**256 - 1) happens here
