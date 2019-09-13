pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		uint x = 0;
		while (x == 0) {
			++x;
			break;
			++x;
		}
		// Assertion is safe but break is unsupported for now
		// so knowledge is erased.
		assert(x == 1);
	}
}
// ----
// Warning: (128-131): Unreachable code.
// Warning: (224-238): Assertion violation happens here
