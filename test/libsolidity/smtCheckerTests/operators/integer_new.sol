contract C {
	function f() public pure {
		uint[] memory x = new uint[](0);
		assert(x.length == 0);
	}
	function g() public pure {
		uint[] memory x = new uint[](3);
		assert(x.length == 3);
		assert(x[0] == 0);
		assert(x[1] == 0);
		assert(x[2] == 0);
	}
	function h() public pure {
		uint[] memory x = new uint[](3);
		assert(x.length == 3);
		x[0] = 0x12;
		x[1] = 0x34;
		assert(x[0] == 0x12);
		assert(x[1] == 0x34);
		// This should be an out-of-bounds assertion.
		x[5] = 0xff;
		assert(x[5] == 0xff);
	}
	function h(uint size) public pure {
		uint[] memory x = new uint[](size);
		assert(x.length == size);
		require(size >= 2);
		x[0] = 0x12;
		x[1] = 0x34;
		assert(x[0] == 0x12);
		assert(x[1] == 0x34);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6368: (474-478): CHC: Out of bounds access happens here.\nCounterexample:\n\nx = [18, 52, 0]\n\nTransaction trace:\nC.constructor()\nC.h()
// Warning 6368: (496-500): CHC: Out of bounds access happens here.\nCounterexample:\n\nx = [18, 52, 0]\n\nTransaction trace:\nC.constructor()\nC.h()
