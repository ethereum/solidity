contract C {
	function f() public pure {
		assert(bytes1("") & ("") == bytes1(0)); // should hold
		assert(bytes1(0xAA) & bytes1(0x55) == bytes1(0)); // should hold
		assert(bytes1(0xFF) & bytes1(0xAA) == bytes1(0xAA)); // should hold
		assert(bytes1(0xFF) & bytes1(0xAA) == bytes1(0)); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (237-285): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
