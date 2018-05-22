contract test {
	uint256 a = 2.2e10;
	uint256 b = .5E10;
	uint256 c = 2 + 2.;
}
// ----
// ParserError: (76-77): Expected identifier but got ';'
