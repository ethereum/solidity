contract test {
	function fun() {
		mapping(var=>bytes32) d;
	}
}
// ----
// ParserError 1005: (44-47): Expected elementary type name or identifier for mapping key type
