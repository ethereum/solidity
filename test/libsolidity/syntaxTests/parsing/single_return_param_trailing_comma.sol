contract test {
	function() returns (uint a,) {}
}
// ----
// ParserError: (43-44): Unexpected trailing comma in parameter list.
