pragma abicoder               v2;
contract C {
	function f(uint256[][] calldata x) external returns (uint256) {
		return x[0].length;
	}
}
// ====
// EVMVersion: >=byzantium
// compileViaYul: also
// revertStrings: debug
// ----
// f(uint256[][]): 0x20, 1, 0x20, 0x0100000000000000000000 -> FAILURE, hex"08c379a0", 0x20, 28, "Invalid calldata tail length"
