contract C {
	function f(uint256 start, uint256 end, uint256[] calldata arr) external pure {
		arr[start:end];
	}
}
// ====
// EVMVersion: >=byzantium
// revertStrings: debug
// compileViaYul: also
// ----
// f(uint256,uint256,uint256[]): 2, 1, 0x80, 3, 1, 2, 3 -> FAILURE, hex"08c379a0", 0x20, 22, "Slice starts after end"
// f(uint256,uint256,uint256[]): 1, 5, 0x80, 3, 1, 2, 3 -> FAILURE, hex"08c379a0", 0x20, 28, "Slice is greater than length"
