pragma experimental SMTChecker;

contract C
{
	uint x;

	modifier m {
		require(x > 0);
		_;
	}

	function f() m public view {
		assert(x > 0);
	}
}
