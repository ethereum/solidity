contract test {
	uint256 stateVar;
	function functionName(bytes32 input) public returns (bytes32 out) {}
}
// ----
// Warning: (58-71): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (89-100): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (36-104): Function state mutability can be restricted to pure
