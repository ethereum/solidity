pragma experimental SMTChecker;
contract C
{
	uint a;
	function f(uint x) public {
		uint y;
		a = (y = x);
	}
	function g() public {
		f(1);
		f(42);
		assert(a > 1);
	}
}

