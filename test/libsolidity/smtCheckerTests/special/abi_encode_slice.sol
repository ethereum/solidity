pragma experimental SMTChecker;
contract C {
	function f(bytes calldata data) external pure returns (bytes memory) {
		return abi.encode(bytes(data[:32]));
	}
}
// ----
// Warning 4588: (126-154): Assertion checker does not yet implement this type of function call.
// Warning 4588: (126-154): Assertion checker does not yet implement this type of function call.
