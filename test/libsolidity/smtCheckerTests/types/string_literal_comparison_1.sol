contract C {
	function f(bytes32 _x) public pure {
		require(_x == "test");
		bytes32 y = _x;
		bytes32 z = _x;
		assert(z == "test");
		assert(y == "testx");
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (137-157): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
