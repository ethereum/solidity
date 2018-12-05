pragma experimental SMTChecker;

contract C
{
	function f(uint x) public pure {
		require(x < 100);
		for(uint i = 0; i < 10; ++i) {
			// Overflows due to resetting x.
			x = x + 1;
		}
		// The assertion is true but x is touched and reset.
		assert(x > 0);
	}
}
// ----
// Warning: (176-181): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (172-181): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (244-257): Assertion violation happens here
