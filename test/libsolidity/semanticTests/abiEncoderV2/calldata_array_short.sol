pragma abicoder v2;

contract C {
	function f(uint[] calldata) public {}
}
// ====
// revertStrings: debug
// ----
// f(uint256[]): 0x20, 0 ->
// f(uint256[]): 0x20, 1 -> FAILURE, hex"08c379a0", 0x20, 0x2b, "ABI decoding: invalid calldata a", "rray stride"
// f(uint256[]): 0x20, 2 -> FAILURE, hex"08c379a0", 0x20, 0x2b, "ABI decoding: invalid calldata a", "rray stride"
