pragma experimental SMTChecker;

contract C
{
	function f() public view {
		assert(gasleft() > 0);
		uint g = gasleft();
		assert(g < gasleft());
		assert(g >= gasleft());
	}
}
// ----
// Warning 4661: (76-97): Assertion violation happens here
// Warning 4661: (123-144): Assertion violation happens here
