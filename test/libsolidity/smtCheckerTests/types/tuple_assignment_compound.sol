pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		uint a = 1;
		uint b = 3;
		a += ((((b))));
		assert(a == 3);
	}
}
// ----
// Warning: (122-136): Assertion violation happens here
