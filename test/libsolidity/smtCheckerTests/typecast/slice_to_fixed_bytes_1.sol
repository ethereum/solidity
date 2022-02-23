contract C {
	function fromSlice(bytes calldata c) external pure returns (bytes32) {
		return bytes32(c[0:33]);
	}
}
// ----
