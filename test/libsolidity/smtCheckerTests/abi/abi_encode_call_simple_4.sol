abstract contract D {
	function no(uint a, uint b) external virtual;
}

contract C {
	function callMeMaybe(uint a, uint b) external {}

	function abiEncodeSimple(uint x, uint y, uint z) public pure {
		require(x == y);
		bytes memory b1 = abi.encodeCall(D.no, (x, z));
		bytes memory b2 = abi.encodeCall(D.no, (y, z));
		assert(b1.length == b2.length); // should hold
		assert(b1[0] == b2[0]); // should hold

		bytes memory b3 = abi.encodeCall(D.no, (3, z));
		assert(b1.length == b3.length); // should hold but we don't compute the length precisely
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6368: (377-382): CHC: Out of bounds access happens here.
// Warning 6368: (386-391): CHC: Out of bounds access happens here.
// Warning 6328: (462-492): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
