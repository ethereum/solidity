contract C {
	function f() public pure returns (uint256 x, bytes32 b, uint256 y) {
		(x, (b, y)) = (0x4d, ("abc", 0x2a));
	}
}
// ----
// f() -> 0x4d, 0x6162630000000000000000000000000000000000000000000000000000000000, 0x2a
