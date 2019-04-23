contract test {
	function(uint a,) {}
}
// ----
// ParserError: (32-33): Unexpected trailing comma in parameter list.
// ParserError: (38-39): Expected pragma, import directive or contract/interface/library definition.
