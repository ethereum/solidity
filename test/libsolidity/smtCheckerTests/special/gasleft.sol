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
// Warning 6328: (76-97): Assertion violation happens here
// Warning 6328: (123-144): Assertion violation happens here
