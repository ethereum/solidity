pragma experimental SMTChecker;

contract C {
	function g() internal pure returns (bytes32, bytes16) {
		return ("test", "testz");
	}

	function f(bytes32 _x) public pure {
		require(_x == "test");
		bytes32 y;
		bytes16 z;
		(y, z) = g();
		assert(_x == y);
		assert(_x == z);
	}
}
// ----
// Warning: (261-276): Assertion violation happens here
