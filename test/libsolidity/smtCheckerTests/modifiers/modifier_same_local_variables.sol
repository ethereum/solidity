pragma experimental SMTChecker;

contract C
{
	modifier m {
		uint x = 2;
		_;
	}

	function f(uint x) m public pure {
		assert(x == 2);
	}
}
// ----
// Warning: (121-135): Assertion violation happens here
