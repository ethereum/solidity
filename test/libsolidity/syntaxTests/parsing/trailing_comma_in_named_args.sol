contract test {
	function test1(uint a, uint b, uint c) public pure returns (uint r) { r = a * 100 + b * 10 + c * 1; }
	function test2() public pure returns (uint r) { r = test1({a: 1, b: 2, c: 3, }); }
}
// ----
