pragma experimental SMTChecker;

contract Base {
	uint x;
	uint private t;
}

contract C is Base {

	uint private z;
	function f(uint y) public {
		require(x < 10);
		require(y < 100);
		z = x + y;
		assert(z < 150);
	}
}
// ----
