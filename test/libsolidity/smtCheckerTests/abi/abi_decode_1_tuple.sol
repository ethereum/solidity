pragma experimental SMTChecker;
contract C {
	function f(bytes calldata data) external pure returns (uint256[] memory) {
		return abi.decode(data, (uint256[]));
	}
}
// ----
// Warning 8364: (148-157): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (147-158): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (148-157): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (147-158): Assertion checker does not yet implement type type(uint256[] memory)
