contract test {
	uint256 stateVariable1;
	function fun(uint256 arg1) public { uint256 y; y = arg1; }
}
// ----
// Warning: Function state mutability can be restricted to pure
