contract test {
	function f() {
		fixed a = 1.0x2;
	}
}
// ----
// ParserError: (44-47): Expected primary expression.
