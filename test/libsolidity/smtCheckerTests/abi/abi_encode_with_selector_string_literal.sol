contract C {
	function abiEncodeStringLiteral(bytes4 sel) public pure {
		bytes memory b1 = abi.encodeWithSelector(sel, "");
		bytes memory b2 = abi.encodeWithSelector(sel, "");

		assert(b1.length == b2.length); // should hold

		bytes memory b3 = abi.encodeWithSelector(sel, bytes(""));
		assert(b1.length == b3.length); // should hold

		bytes memory b4 = abi.encodeWithSelector(sel, bytes24(""));
		assert(b1.length == b4.length); // should fail

		bytes memory b5 = abi.encodeWithSelector(sel, string(""));
		assert(b1.length == b5.length); // should hold, but currently fails due to abstraction

		bytes memory b6 = abi.encodeWithSelector(0xcafecafe, bytes24(""));
		assert(b4.length == b6.length); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (403-433): CHC: Assertion violation happens here.\nCounterexample:\n\nsel = 0x0\nb5 = []\nb6 = []\n\nTransaction trace:\nC.constructor()\nC.abiEncodeStringLiteral(0x0)
// Warning 6328: (514-544): CHC: Assertion violation happens here.\nCounterexample:\n\nsel = 0x0\nb6 = []\n\nTransaction trace:\nC.constructor()\nC.abiEncodeStringLiteral(0x0)
// Warning 6328: (673-703): CHC: Assertion violation happens here.\nCounterexample:\n\nsel = 0x0\nb6 = []\n\nTransaction trace:\nC.constructor()\nC.abiEncodeStringLiteral(0x0)
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
