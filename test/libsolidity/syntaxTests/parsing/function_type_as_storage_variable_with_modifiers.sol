contract test {
	function (uint, uint) modifier1() returns (uint) f1;
}
// ----
// ParserError: (48-49): Expected ';' but got '('
