contract C {
	function abiEncodeHash(uint a, uint b) public pure {
		require(a == b);
		bytes memory b1 = abi.encode(a, a, a, a);
		bytes memory b2 = abi.encode(b, a, b, a);
		assert(keccak256(b1) == keccak256(b2));
	}
}
// ====
// SMTEngine: all
// ----
