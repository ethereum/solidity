contract C {
	function callMeMaybe(uint a, uint b) external {}

	function abiEncodeSimple(uint x, uint y, uint z) public view {
		require(x == y);
		function (uint, uint) external f = this.callMeMaybe;
		bytes memory b1 = abi.encodeCall(f, (x, z));
		bytes memory b2 = abi.encodeCall(f, (y, z));
		assert(b1.length == b2.length); // should hold
		assert(b1[0] == b2[0]); // should hold

		bytes memory b3 = abi.encodeCall(this.callMeMaybe, (3, z));
		assert(b1.length == b3.length); // should hold, but we don't encode the length computation precisely
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6368: (354-359): CHC: Out of bounds access happens here.
// Warning 6368: (363-368): CHC: Out of bounds access happens here.
// Warning 6328: (451-481): CHC: Assertion violation happens here.
