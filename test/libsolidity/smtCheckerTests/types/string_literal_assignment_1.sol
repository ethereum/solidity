pragma experimental SMTChecker;

contract C {
	function f(bytes32 _x) public pure {
		require(_x == "test");
		bytes32 y = "test";
		bytes16 z = "testz";
		assert(_x == y);
		assert(_x == z);
	}
}
// ----
// Warning: (175-190): Assertion violation happens here
