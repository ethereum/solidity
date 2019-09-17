pragma experimental SMTChecker;

contract C
{
	function f(uint x, bool b) public pure {
		require(x < 10);
		while (x < 10) {
			if (b)
				++x;
			else {
				break;
			}
		}
		// Fails because the loop might break.
		assert(x >= 10);
	}
}
// ----
// Warning: (218-233): Assertion violation happens here
