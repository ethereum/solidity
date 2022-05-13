pragma abicoder v1;
contract C {
	enum E {X, Y}
	function f(E[] calldata arr) external {
		arr[1];
	}
}
// ====
// EVMVersion: >=byzantium
// revertStrings: debug
// ABIEncoderV1Only: true
// compileViaYul: false
// ----
// f(uint8[]): 0x20, 2, 3, 3 -> FAILURE, hex"08c379a0", 0x20, 17, "Enum out of range"
