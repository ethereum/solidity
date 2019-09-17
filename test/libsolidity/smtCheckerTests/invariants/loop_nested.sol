pragma experimental SMTChecker;

contract Simple {
	function f() public pure {
		uint x = 10;
		uint y;
		while (y < x)
		{
			++y;
			x = 0;
			while (x < 10)
				++x;
			assert(x == 10);
		}
		assert(y == x);
	}
}
