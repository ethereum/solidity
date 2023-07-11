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
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
