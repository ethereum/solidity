pragma abicoder               v2;
contract C {
	function f(uint256[][] calldata a) external returns (uint) {
		return 42;
	}
}
// ====
// EVMVersion: >=byzantium
// revertStrings: debug
// ----
// f(uint256[][]): 0x20, 1 -> FAILURE, hex"08c379a0", 0x20, 43, "ABI decoding: invalid calldata a", "rray stride"
