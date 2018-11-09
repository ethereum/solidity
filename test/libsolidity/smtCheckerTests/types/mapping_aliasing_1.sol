pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) a;
	mapping (uint => uint) b;

	function f() public {
		require(a[1] == b[1]);
		mapping (uint => uint) storage c = a;
		c[1] = 2;
		// False negative! Needs aliasing.
		assert(a[1] == b[1]);
	}
}
