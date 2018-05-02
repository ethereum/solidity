contract test {
	function f() {
		fixed a = 1.0x2;
	}
}
// ----
// ParserError: (44-44): Expected primary expression.
