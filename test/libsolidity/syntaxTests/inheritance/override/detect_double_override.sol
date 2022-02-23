contract X {
	int public override override testvar;
	function test() internal override override returns (uint256);
}
// ----
// ParserError 9125: (34-42): Override already specified.
// ParserError 1827: (87-95): Override already specified.
