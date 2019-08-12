pragma experimental SMTChecker;

contract C {
	function f(bytes32 _x) public pure {
		require(_x == "test");
		(bytes32 y, bytes16 z) = ("test", "testz");
		assert(_x == y);
		assert(_x == z);
	}
}
// ----
// Warning: (176-191): Assertion violation happens here
