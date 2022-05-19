pragma abicoder v2;

contract C {
	function h(uint[][] calldata a) public {
		abi.encode(a);
	}
	struct S { uint[] x; }
	function f(S calldata a) public {
		abi.encode(a);
	}
}
// ====
// revertStrings: debug
// ----
// h(uint256[][]): 0x20, 1, 0x20, 0 ->
// h(uint256[][]): 0x20, 1, 0x20, 1 -> FAILURE, hex"08c379a0", 0x20, 0x1e, "Invalid calldata access stride"
// h(uint256[][]): 0x20, 1, 0x20, 2 -> FAILURE, hex"08c379a0", 0x20, 0x1e, "Invalid calldata access stride"
// h(uint256[][]): 0x20, 1, 0x20, 3 -> FAILURE, hex"08c379a0", 0x20, 0x1e, "Invalid calldata access stride"
// f((uint256[])): 0x20, 0x20, 0 ->
// f((uint256[])): 0x20, 0x20, 1 -> FAILURE, hex"08c379a0", 0x20, 0x1e, "Invalid calldata access stride"
// f((uint256[])): 0x20, 0x20, 2 -> FAILURE, hex"08c379a0", 0x20, 0x1e, "Invalid calldata access stride"
// f((uint256[])): 0x20, 0x20, 3 -> FAILURE, hex"08c379a0", 0x20, 0x1e, "Invalid calldata access stride"
