contract test {
	function (uint, uint) modifier1() returns (uint) f1;
}
// ----
// ParserError: (66-66): Expected token LBrace got 'Identifier'
