pragma experimental SMTChecker;

contract C
{
	function f(uint x, bool b) public pure {
		require(x < 10);
		for (; x < 10; ) {
			if (b) {
				x = 20;
				continue;
			}
			++x;
		}
		assert(x > 15);
	}
}
// ----
// Warning: (185-199): Assertion violation happens here
