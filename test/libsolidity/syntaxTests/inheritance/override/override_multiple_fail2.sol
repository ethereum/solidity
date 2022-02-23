contract A {
	function foo() internal returns (uint256);
}
contract X {
	function test() internal override(,) returns (uint256);
}
// ----
// ParserError 2314: (107-108): Expected identifier but got ','
