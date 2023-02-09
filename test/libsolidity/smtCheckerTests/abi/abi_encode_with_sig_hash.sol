contract C {
	function abiEncodeHash(string memory sig, uint a, uint b) public pure {
		require(a == b);
		bytes memory b1 = abi.encodeWithSignature(sig, a, a, a, a);
		bytes memory b2 = abi.encodeWithSignature(sig, b, a, b, a);
		assert(keccak256(b1) == keccak256(b2));

		bytes memory b3 = abi.encodeWithSelector("f()", a, a, a, a);
		assert(keccak256(b1) == keccak256(b3)); // should fail
		assert(keccak256(b1) != keccak256(b3)); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (337-375): CHC: Assertion violation happens here.
// Warning 6328: (394-432): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
