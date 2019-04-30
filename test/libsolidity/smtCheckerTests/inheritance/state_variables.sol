pragma experimental SMTChecker;

contract Base {
	uint x;
	uint z;
	uint private t;
}

contract C is Base {
	function f(uint y) public {
		require(x < 10);
		require(y < 100);
		z = x + y;
		assert(z < 150);
	}
}
// ----
