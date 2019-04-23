contract test {
	function() returns (uint a,) {}
}
// ----
// ParserError: (43-44): Unexpected trailing comma in parameter list.
// ParserError: (49-50): Expected pragma, import directive or contract/interface/library definition.
