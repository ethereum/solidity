contract C {
	function f(bytes calldata data) external pure returns (uint256[] memory) {
		return abi.decode(data, (uint256[]));
	}
}
// ====
// SMTEngine: all
// ----
