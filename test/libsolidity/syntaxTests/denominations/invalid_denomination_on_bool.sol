contract C {
	bool constant x = true ether;
}
// ----
// ParserError 2314: (37-42): Expected ';' but got 'ether'
