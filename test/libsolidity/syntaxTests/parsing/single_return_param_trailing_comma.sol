contract test {
	function() returns (uint a,) {}
}
// ----
// ParserError: (43-43): Unexpected trailing comma in parameter list.
