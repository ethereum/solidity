contract C {
	function abiEncodeStringLiteral(string memory sig) public pure {
		bytes memory b1 = abi.encodeWithSignature(sig, "");
		bytes memory b2 = abi.encodeWithSignature(sig, "");
		assert(b1.length == b2.length); // should hold

		bytes memory b3 = abi.encodeWithSignature(sig, bytes(""));
		assert(b1.length == b3.length); // should hold

		bytes memory b4 = abi.encodeWithSignature(sig, bytes24(""));
		assert(b1.length == b4.length); // should fail

		bytes memory b5 = abi.encodeWithSignature(sig, string(""));
		assert(b1.length == b5.length); // should hold, but currently fails due to abstraction

		bytes memory b6 = abi.encodeWithSelector("f()", bytes24(""));
		assert(b4.length == b6.length); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (413-443): CHC: Assertion violation happens here.\nCounterexample:\n\nb5 = []\nb6 = []\n\nTransaction trace:\nC.constructor()\nC.abiEncodeStringLiteral(sig) -- counterexample incomplete; parameter name used instead of value
// Warning 6328: (525-555): CHC: Assertion violation happens here.\nCounterexample:\n\nb6 = []\n\nTransaction trace:\nC.constructor()\nC.abiEncodeStringLiteral(sig) -- counterexample incomplete; parameter name used instead of value
// Warning 6328: (679-709): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.abiEncodeStringLiteral(sig) -- counterexample incomplete; parameter name used instead of value
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
