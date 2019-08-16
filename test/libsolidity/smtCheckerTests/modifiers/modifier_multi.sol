pragma experimental SMTChecker;

contract C
{
	uint x;

	modifier m {
		require(x > 0);
		require(x < 10000);
		_;
	}

	modifier n {
		x = x + 1;
		_;
		assert(x > 2);
		assert(x > 8);
	}

	function f() m n public {
		x = x + 1;
	}

	function g(uint _x) public {
		x = _x;
	}
}
// ----
// Warning: (170-183): Assertion violation happens here
