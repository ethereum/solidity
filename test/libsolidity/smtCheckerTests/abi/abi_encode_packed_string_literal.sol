contract C {
	function abiencodePackedStringLiteral() public pure {
		bytes memory b1 = abi.encodePacked("");
		bytes memory b2 = abi.encodePacked("");
		assert(b1.length == b2.length); // should hold

		bytes memory b3 = abi.encodePacked(bytes(""));
		assert(b1.length == b3.length); // should hold

		bytes memory b4 = abi.encodePacked(bytes24(""));
		assert(b1.length == b4.length); // should fail

		bytes memory b5 = abi.encodePacked(string(""));
		assert(b1.length == b5.length); // should hold, but currently fails due to abstraction

		bytes memory b6 = abi.encode("");
		assert(b1.length == b6.length); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (354-384): CHC: Assertion violation happens here.\nCounterexample:\n\nb5 = []\nb6 = []\n\nTransaction trace:\nC.constructor()\nC.abiencodePackedStringLiteral()
// Warning 6328: (454-484): CHC: Assertion violation happens here.\nCounterexample:\n\nb6 = []\n\nTransaction trace:\nC.constructor()\nC.abiencodePackedStringLiteral()
// Warning 6328: (580-610): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.abiencodePackedStringLiteral()
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
