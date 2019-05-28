pragma experimental SMTChecker;

contract Base1 {
	uint x;
	uint private t;
}

contract Base2 is Base1 {
	uint z;
	uint private t;
}

contract C is Base2 {
	function f(uint y) public {
		require(x < 10);
		require(y < 100);
		z = x + y;
		assert(z < 150);
	}
}
// ----
