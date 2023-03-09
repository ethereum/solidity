contract C {
	function g() internal pure returns (bytes32, bytes16) {
		return ("test", "testz");
	}

	function f(bytes32 _x) public pure {
		require(_x == "test");
		(bytes32 y, bytes16 z) = g();
		assert(_x == y);
		assert(_x == z);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (218-233): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
