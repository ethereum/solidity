contract test {
	function a(uint a, uint b, uint c) returns (uint r) { r = a * 100 + b * 10 + c * 1; }
	function b() returns (uint r) { r = a({a: , b: , c: }); }
}
// ----
// ParserError: (146-147): Expected primary expression.
