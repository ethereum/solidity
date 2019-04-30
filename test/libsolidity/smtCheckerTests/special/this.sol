pragma experimental SMTChecker;

contract C
{
	function f(address a) public view {
		assert(a == address(this));
	}
}
// ----
// Warning: (85-111): Assertion violation happens here
