contract C {
	function f(bytes calldata data) external pure returns (bytes memory) {
		return abi.encode(bytes(data[:32]));
	}
}
// ====
// SMTEngine: all
// ----
