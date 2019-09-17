pragma experimental SMTChecker;

contract C
{
	function f(uint x, bool b) public pure {
		require(x < 10);
		while (x < 10) {
			if (b)
				++x;
			else {
				x = 20;
				break;
			}
		}
		// Assertion is safe but break is unsupported for now
		// so knowledge is erased.
		assert(x >= 10);
	}
}
// ----
// Warning: (274-289): Assertion violation happens here
