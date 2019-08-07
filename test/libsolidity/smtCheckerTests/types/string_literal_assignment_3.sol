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
// Warning: (186-201): Assertion violation happens here
