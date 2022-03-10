contract C {
	function removeSignature(bytes memory x) internal pure returns (bytes memory r) {
		r = new bytes(x.length - 4);
		for (uint i = 0; i < x.length - 4; ++i)
			r[i] = x[i + 4];
	}
	function g(bytes2, bytes2) public {}
	function h(uint16, uint16) public {}
	function f() public returns (bytes memory) {
		uint16 x = 0x1234;
		return removeSignature(abi.encodeCall(this.g, (0x1234, bytes2(x))));
	}
	function f2() public returns (bytes memory) {
		bytes2 x = 0x1234;
		return removeSignature(abi.encodeCall(this.h, (0x1234, uint16(x))));
	}
}
// ====
// compileViaYul: also
// ----
// f() -> 0x20, 0x40, 0x1234, 0x1234000000000000000000000000000000000000000000000000000000000000
// f2() -> 0x20, 0x40, 0x1234, 0x1234
