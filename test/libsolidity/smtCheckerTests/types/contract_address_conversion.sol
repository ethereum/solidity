pragma experimental SMTChecker;

contract C
{
	function f(C c, address a) public pure {
		assert(address(c) == a);
	}
}
// ----
// Warning: (90-113): Assertion violation happens here
