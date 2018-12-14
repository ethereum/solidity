pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) a;
	mapping (uint => uint) b;

	function f() public {
		require(a[1] == b[1]);
		a[1] = 2;
		mapping (uint => uint) storage c = a;
		assert(c[1] == 2);
		// False negative! Needs aliasing.
		assert(a[1] == b[1]);
	}
}
// ----
// Warning: (261-281): Assertion violation happens here
