contract C {
	function f() public pure {
		bytes32 x = 0x00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff;
		bytes1 z = 0x00;
		bytes1 o = 0xff;
		assert(x[0] == z);
		assert(x[31] == o);
		assert(x[0] == x[22]);
		assert(x[0] == x[23]);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (231-252): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 450552876409790643671482431940419874915447411150352389258589821042463539455\nz = 0\no = 255\n\nTransaction trace:\nC.constructor()\nC.f()
