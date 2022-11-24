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
// Warning 6328: (337-375): CHC: Assertion violation happens here.\nCounterexample:\n\na = 0\nb = 0\nb1 = [0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d]\nb2 = [0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d]\n\nTransaction trace:\nC.constructor()\nC.abiEncodeHash(sig, 0, 0)
// Warning 6328: (394-432): CHC: Assertion violation happens here.\nCounterexample:\n\na = 0\nb = 0\n\nTransaction trace:\nC.constructor()\nC.abiEncodeHash(sig, 0, 0)
