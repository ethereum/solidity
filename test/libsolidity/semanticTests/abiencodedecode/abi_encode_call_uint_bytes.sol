contract C {
	function removeSignature(bytes calldata x) external pure returns (bytes memory) {
		return x[4:];
	}
	function g(bytes2, bytes2, bytes2) public {}
	function h(uint16, uint16) public {}
	function f() public returns (bytes memory) {
		uint16 x = 0x1234;
		return this.removeSignature(abi.encodeCall(this.g, (0x1234, "ab", bytes2(x))));
	}
	function f2() public returns (bytes memory) {
		bytes2 x = 0x1234;
		return this.removeSignature(abi.encodeCall(this.h, (0x1234, uint16(x))));
	}
}
// ====
// EVMVersion: >homestead
// ----
// f() -> 0x20, 0x60, 0x1234000000000000000000000000000000000000000000000000000000000000, 0x6162000000000000000000000000000000000000000000000000000000000000, 0x1234000000000000000000000000000000000000000000000000000000000000
// f2() -> 0x20, 0x40, 0x1234, 0x1234
