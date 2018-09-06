contract test {
	uint256 stateVar;
	function functionName(bytes20 arg1, address addr) public view returns (int id) { }
}
// ----
// Warning: (58-70): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (72-84): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (107-113): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (36-118): Function state mutability can be restricted to pure
