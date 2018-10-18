pragma experimental SMTChecker;

contract C
{
	function f() public view {
		assert(gasleft() > 0);
	}
}
// ----
// Warning: (76-97): Assertion violation happens here
