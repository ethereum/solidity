contract test {
	function fun(uint a) returns(uint r) { return a; }
	function fun(uint a, uint b) returns(uint r) { return a + b; }
}
// ----
// Warning: (17-67): No visibility specified. Defaulting to "public". 
// Warning: (69-131): No visibility specified. Defaulting to "public". 
// Warning: (17-67): Function state mutability can be restricted to pure
// Warning: (69-131): Function state mutability can be restricted to pure
