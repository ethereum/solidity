pragma experimental SMTChecker;

contract C
{
	uint[][] a;
	function f() public {
		require(a[2][3] == 4);
		delete a;
		assert(a[2][3] == 0);
	}
}
