pragma experimental SMTChecker;

contract C
{
	uint x;
	function f() internal {
		require(x < 10000);
		x = x + 1;
	}
	function g(bool b) public {
		x = 0;
		if (b)
			f();
		else
			f();
		assert(x == 1);
	}
}
