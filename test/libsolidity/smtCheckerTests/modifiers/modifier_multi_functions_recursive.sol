pragma experimental SMTChecker;

contract C
{
	modifier m(uint a, uint b) {
		require(g(a, b));
		_;
	}

	function g(uint a, uint b) m(a, b) internal pure returns (bool) {
		return a > b;
	}

	function f(uint x) m(x, 0) public pure {
		assert(x > 0);
		assert(x > 1);
	}
}
// ----
// Warning: (86-93): Assertion checker does not support recursive function calls.
// Warning: (86-93): Internal error: Expression undefined for SMT solver.
// Warning: (86-93): Assertion checker does not support recursive function calls.
// Warning: (86-93): Internal error: Expression undefined for SMT solver.
// Warning: (253-266): Assertion violation happens here
