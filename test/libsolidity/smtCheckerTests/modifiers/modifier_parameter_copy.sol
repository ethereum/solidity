pragma experimental SMTChecker;

contract C
{
	modifier m(uint x) {
		x == 2;
		_;
	}

	function f(uint x) m(x) public pure {
		assert(x == 2);
	}
}
// ----
// Warning: (128-142): Assertion violation happens here
