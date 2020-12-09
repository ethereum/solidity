pragma experimental SMTChecker;

contract C {
	function f(bytes32 _x) public pure {
		require(_x == "test");
		bytes32 y;
		bytes16 z;
		(y, z) = ("test", "testz");
		assert(_x == y);
		assert(_x == z);
	}
}
// ----
// Warning 6328: (186-201): CHC: Assertion violation happens here.\nCounterexample:\n\n_x = 52647538817385212172903286807934654968315727694643370704309751478220717293568\n\n\nTransaction trace:\nconstructor()\nf(52647538817385212172903286807934654968315727694643370704309751478220717293568)
