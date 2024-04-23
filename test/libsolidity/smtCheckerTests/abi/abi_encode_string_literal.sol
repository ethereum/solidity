contract C {
	function abiEncodeStringLiteral() public pure {
		bytes memory b1 = abi.encode("");
		bytes memory b2 = abi.encode("");
		// should hold
		assert(b1.length == b2.length);

		bytes memory b3 = abi.encode(bytes(""));
		assert(b1.length == b3.length); // should hold

		bytes memory b4 = abi.encode(bytes24(""));
		assert(b1.length == b4.length); // should fail

		bytes memory b5 = abi.encode(string(""));
		assert(b1.length == b5.length); // should hold, but currently fails due to abstraction
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (326-356): CHC: Assertion violation happens here.\nCounterexample:\n\nb5 = []\n\nTransaction trace:\nC.constructor()\nC.abiEncodeStringLiteral()
// Warning 6328: (420-450): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.abiEncodeStringLiteral()
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
