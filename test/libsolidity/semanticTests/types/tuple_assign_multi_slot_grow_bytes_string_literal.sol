contract C {
	function f() public pure returns (bytes32 b, uint256 x, uint256 y) {
		((b, x), y) = (("abc", 0x2a), 0x4d);
	}
}
// ----
// f() -> 0x6162630000000000000000000000000000000000000000000000000000000000, 0x2a, 0x4d
