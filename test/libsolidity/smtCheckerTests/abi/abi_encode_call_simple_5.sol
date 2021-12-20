abstract contract D {
	function no(uint a, uint b) external virtual;
}

contract C {
	function callMeMaybe(uint a, uint b) external {}

	function abiEncodeSimple(D d, uint x, uint y, uint z) public pure {
		require(x == y);
		bytes memory b1 = abi.encodeCall(d.no, (x, z));
		bytes memory b2 = abi.encodeCall(d.no, (y, z));
		assert(b1.length == b2.length); // should hold
		assert(b1[0] == b2[0]); // should hold

		bytes memory b3 = abi.encodeCall(d.no, (3, z));
		assert(b1.length == b3.length); // should hold but we don't compute the length precisely
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6368: (382-387): CHC: Out of bounds access happens here.
// Warning 6368: (391-396): CHC: Out of bounds access happens here.
// Warning 6328: (467-497): CHC: Assertion violation happens here.
