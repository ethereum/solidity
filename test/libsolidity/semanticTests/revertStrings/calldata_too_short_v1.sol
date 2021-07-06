pragma abicoder v1;
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
// d(bytes): 0x20, 0x01, 0x0000000000000000000000000000000000000000000000000000000000000000 -> FAILURE, hex"08c379a0", 0x20, 18, "Calldata too short"
