contract C {
	function f() public pure returns (bytes32 b, uint256 x, uint256 y, uint256 z) {
		(((b, x), y), z) = ((("abc", 0x2a), 0x4d), 0x63);
	}
}
// ----
// f() -> 0x6162630000000000000000000000000000000000000000000000000000000000, 0x2a, 0x4d, 0x63
