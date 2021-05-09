contract C {
	function f(bytes calldata data) external pure returns (uint256[] memory) {
		return abi.decode(data, (uint256[]));
	}
}
// ====
// SMTEngine: all
// ----
// Warning 8364: (116-125): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (115-126): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (116-125): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (115-126): Assertion checker does not yet implement type type(uint256[] memory)
