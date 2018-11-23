contract test {
	function f() {
		fixed a = 1.0x2;
	}
}
// ----
// ParserError: (44-47): Identifier-start is not allowed at end of a number.
