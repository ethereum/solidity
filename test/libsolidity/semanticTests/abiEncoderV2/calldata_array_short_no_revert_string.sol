contract C {
	function f(uint[] calldata) public {}
}
// ----
// f(uint256[]): 0x20, 0 ->
// f(uint256[]): 0x20, 1 -> FAILURE
// f(uint256[]): 0x20, 2 -> FAILURE

