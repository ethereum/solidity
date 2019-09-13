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
