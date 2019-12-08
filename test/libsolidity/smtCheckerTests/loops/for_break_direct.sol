pragma experimental SMTChecker;

contract C
{
	function f(uint x) public pure {
		for (x = 0; x < 10; ++x)
			break;
		assert(x == 0);
	}
}
// ----
// Warning: (102-105): Unreachable code.
