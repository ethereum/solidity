contract test {
	uint256 stateVar;
	function functionName(bytes32 input) returns (bytes32 out) {}
}
// ----
// Warning: (36-97): No visibility specified. Defaulting to "public". 
// Warning: (58-71): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (82-93): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (36-97): Function state mutability can be restricted to pure
