pragma experimental SMTChecker;

contract C
{
	uint x;

	modifier m {
		require(x > 0);
		_;
	}

	function f() m public {
		assert(x > 0);
		x = x + 1;
	}
}
// ----
// Warning: (145-150): Overflow (resulting value larger than 2**256 - 1) happens here
