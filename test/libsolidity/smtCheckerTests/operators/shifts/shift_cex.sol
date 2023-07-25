contract C {
	function left() public pure {
		uint x = 0x4266;
		assert(x << 0x0 == 0x4266);
		// Fails because the above is true.
		assert(x << 0x0 == 0x4268);
	}

	function right() public pure {
		uint x = 0x4266;
		assert(x >> 0x0 == 0x4266);
		// Fails because the above is true.
		assert(x >> 0x0 == 0x4268);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: no
// ----
// Warning 6328: (133-159): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 16998\n\nTransaction trace:\nC.constructor()\nC.left()
// Warning 6328: (286-312): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 16998\n\nTransaction trace:\nC.constructor()\nC.right()
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
