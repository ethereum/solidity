pragma experimental SMTChecker;

contract C
{
	uint x;

	modifier m {
		require(x > 0);
		_;
		// Fails because of overflow behavior.
		assert(x > 1);
	}

	function f() m public {
		assert(x > 0);
		x = x + 1;
	}

	function g(uint _x) public {
		x = _x;
	}
}
// ----
// Warning: (203-208): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (136-149): Assertion violation happens here
