pragma experimental SMTChecker;

contract C
{
	function g() public pure {
		(uint x, uint y) = (2, 4);
		assert(x == 2);
		assert(y == 4);
	}
}
// ----
// Warning: (76-101): Assertion checker does not yet support such variable declarations.
// Warning: (105-119): Assertion violation happens here
