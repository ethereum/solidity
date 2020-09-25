pragma experimental SMTChecker;

contract C {
	function f() public pure {
		bytes memory x = new bytes(0);
		assert(x.length == 0);
	}
	function g() public pure {
		bytes memory x = new bytes(3);
		assert(x.length == 3);
		assert(x[0] == 0);
		assert(x[1] == 0);
		assert(x[2] == 0);
	}
	function h() public pure {
		bytes memory x = new bytes(3);
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
		bytes memory x = new bytes(size);
		assert(x.length == size);
		require(size >= 2);
		x[0] = 0x12;
		x[1] = 0x34;
		assert(x[0] == 0x12);
		assert(x[1] == 0x34);
	}
}
// ----
