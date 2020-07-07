pragma experimental SMTChecker;

contract C
{
	function f(address a) public view {
		assert(a == address(this));
	}
}
// ----
// Warning 4661: (85-111): Assertion violation happens here
