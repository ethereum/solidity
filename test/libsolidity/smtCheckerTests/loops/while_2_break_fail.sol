pragma experimental SMTChecker;

contract C
{
	function f(uint x) public pure {
		while (x == 0) {
			++x;
			break;
			++x;
		}
		assert(x == 2);
	}
}
// ----
// Warning: (120-123): Unreachable code.
// Warning: (131-145): Assertion violation happens here
