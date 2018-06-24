contract test {
	uint256 stateVar;
	function functionName(bytes20 arg1, address addr) view returns (int id) { }
}
// ----
// Warning: (36-111): No visibility specified. Defaulting to "public". 
// Warning: (58-70): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (72-84): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (100-106): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (36-111): Function state mutability can be restricted to pure
