contract C {
	function abiencodePackedHash(uint a, uint b) public pure {
		require(a == b);
		bytes memory b1 = abi.encodePacked(a, a, a, a);
		bytes memory b2 = abi.encodePacked(b, a, b, a);
		assert(keccak256(b1) == keccak256(b2));

		bytes memory b3 = abi.encode(a, a, a, a);
		assert(keccak256(b1) == keccak256(b3)); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (281-319): CHC: Assertion violation happens here.\nCounterexample:\n\na = 0\nb = 0\n\nTransaction trace:\nC.constructor()\nC.abiencodePackedHash(0, 0)
