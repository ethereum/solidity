contract c {
	modifier mod { if (msg.sender == 0) _ }
}
// ----
// ParserError 2314: (52-53): Expected ';' but got '}'
