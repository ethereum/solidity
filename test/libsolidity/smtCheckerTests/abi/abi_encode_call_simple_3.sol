contract C {
	function callMeMaybe(uint a, uint b) external {}

	function abiEncodeSimple(uint x, uint y, uint z) public pure {
		require(x == y);
		bytes memory b1 = abi.encodeCall(C.callMeMaybe, (x, z));
		bytes memory b2 = abi.encodeCall(C.callMeMaybe, (y, z));
		assert(b1.length == b2.length); // should hold
		assert(b1[0] == b2[0]); // should hold

		bytes memory b3 = abi.encodeCall(C.callMeMaybe, (3, z));
		assert(b1.length == b3.length); // should hold but we don't compute the length precisely
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6368: (323-328): CHC: Out of bounds access happens here.
// Warning 6368: (332-337): CHC: Out of bounds access happens here.
// Warning 6328: (417-447): CHC: Assertion violation happens here.
