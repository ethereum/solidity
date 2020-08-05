contract C {
	function() external virtual fp;
	function() external override fp2;
	function() external override virtual fp3;
}
// ----
// ParserError 2314: (34-41): Expected identifier but got 'virtual'
