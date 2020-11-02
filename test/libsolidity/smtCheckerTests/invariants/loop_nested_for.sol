pragma experimental SMTChecker;

contract Simple {
	function f() public pure {
		uint x;
		uint y;
		for (x = 10; y < x; ++y)
		{
			for (x = 0; x < 10; ++x) {}
			assert(x == 10);
		}
		assert(y == x);
	}
}
// ----
// Warning 6328: (187-201): CHC: Assertion violation might happen here.
// Warning 4661: (187-201): BMC: Assertion violation happens here.
