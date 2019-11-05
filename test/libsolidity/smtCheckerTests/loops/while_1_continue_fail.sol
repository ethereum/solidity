pragma experimental SMTChecker;

contract C
{
	function f(uint x, bool b) public pure {
		require(x < 100);
		while (x < 10) {
			if (b) {
				x = 15;
				continue;
				x = 200;
			}
			x = 20;

		}
		// Fails due to the if.
		assert(x >= 17);
	}
}
// ----
// Warning: (169-176): Unreachable code.
// Warning: (227-242): Assertion violation happens here
