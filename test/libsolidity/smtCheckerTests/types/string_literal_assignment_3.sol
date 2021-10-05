contract C {
	function f(bytes32 _x) public pure {
		require(_x == "test");
		bytes32 y;
		bytes16 z;
		(y, z) = ("test", "testz");
		assert(_x == y);
		assert(_x == z);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (153-168): CHC: Assertion violation happens here.\nCounterexample:\n\n_x = 0x7465737400000000000000000000000000000000000000000000000000000000\ny = 0x7465737400000000000000000000000000000000000000000000000000000000\nz = 0x746573747a0000000000000000000000\n\nTransaction trace:\nC.constructor()\nC.f(0x7465737400000000000000000000000000000000000000000000000000000000)
