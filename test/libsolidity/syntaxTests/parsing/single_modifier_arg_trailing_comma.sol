contract test {
	modifier modTest(uint a,) { _; }
	function(uint a) {}
}
// ----
// ParserError: (40-41): Unexpected trailing comma in parameter list.
