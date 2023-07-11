contract C {
	function f() public pure {
		bytes memory b = bytes(hex"ffff");
		assert(b.length == 2); // should hold
		assert(b[0] == bytes1(uint8(255))); // should hold
		assert(b[1] == bytes1(uint8(100))); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (173-207): CHC: Assertion violation happens here.
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
