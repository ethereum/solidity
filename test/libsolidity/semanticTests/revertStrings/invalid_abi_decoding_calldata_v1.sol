contract C {
	function d(bytes memory _data) public pure returns (uint8) {
		return abi.decode(_data, (uint8));
	}
}
// ====
// EVMVersion: >=byzantium
// revertStrings: debug
// ABIEncoderV1Only: true
// ----
// d(bytes): 0x20, 0x20, 0x0000000000000000000000000000000000000000000000000000000000000000 -> 0
// d(bytes): 0x100, 0x20, 0x0000000000000000000000000000000000000000000000000000000000000000 -> FAILURE, hex"08c379a0", 0x20, 43, "ABI calldata decoding: invalid h", "ead pointer"
// d(bytes): 0x20, 0x100, 0x0000000000000000000000000000000000000000000000000000000000000000 -> FAILURE, hex"08c379a0", 0x20, 43, "ABI calldata decoding: invalid d", "ata pointer"
