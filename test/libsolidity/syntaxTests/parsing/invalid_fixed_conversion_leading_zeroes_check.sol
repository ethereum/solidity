contract test {
	function f() {
		fixed a = 1.0x2;
	}
}
// ----
// ParserError: (44-47): Digit or Identifier-Start not allowed at end of Number.
