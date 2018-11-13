contract test {
	function fun(uint a) public returns(uint r) { return a; }
	function fun(uint a, uint b) public returns(uint r) { return a + b; }
}
// ----
// Warning: (17-74): Function state mutability can be restricted to pure
// Warning: (76-145): Function state mutability can be restricted to pure
