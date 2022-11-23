contract C {
	function abiEncodeSimple(bool t, uint x, uint y, uint z, uint[] memory a, uint[] memory b) public pure {
		require(x == y);
		bytes memory b1 = abi.encode(x, z, a);
		bytes memory b2 = abi.encode(y, z, a);
		assert(b1.length == b2.length);

		bytes memory b3 = abi.encode(y, z, b);
		assert(b1.length == b3.length); // should fail

		bytes memory b4 = abi.encode(t, z, a);
		assert(b1.length == b4.length); // should fail

		bytes memory b5 = abi.encode(y, y, y, y, a, a, a);
		assert(b1.length != b5.length); // should fail
		// Disabled because of nondeterminism in Spacer Z3 4.8.9
		//assert(b1.length == b5.length); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (298-328): CHC: Assertion violation happens here.\nCounterexample:\n\nt = false\nx = 0\ny = 0\nz = 0\nb4 = []\nb5 = []\n\nTransaction trace:\nC.constructor()\nC.abiEncodeSimple(false, 0, 0, 0, a, b)
// Warning 6328: (389-419): CHC: Assertion violation happens here.\nCounterexample:\n\nt = false\nx = 0\ny = 0\nz = 0\nb5 = []\n\nTransaction trace:\nC.constructor()\nC.abiEncodeSimple(false, 0, 0, 0, a, b)
// Warning 6328: (492-522): CHC: Assertion violation happens here.\nCounterexample:\n\nt = false\nx = 0\ny = 0\nz = 0\nb1 = []\nb2 = []\nb5 = []\n\nTransaction trace:\nC.constructor()\nC.abiEncodeSimple(false, 0, 0, 0, a, b)
