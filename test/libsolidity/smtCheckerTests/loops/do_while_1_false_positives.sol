pragma experimental SMTChecker;

contract C
{
	function f(uint x) public pure {
		require(x < 100);
		do {
			// Overflows due to resetting x.
			x = x + 1;
		} while (x < 1000);
		// The assertion is true but we can't infer so
		// because x is touched in the loop.
		assert(x > 0);
	}
}
// ----
// Warning: (150-155): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (269-282): Assertion violation happens here
