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
		assert(x == 1);
	}
}
// ----
// Warning: (128-131): Unreachable code.
