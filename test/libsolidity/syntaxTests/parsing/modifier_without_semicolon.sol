contract c {
	modifier mod { if (msg.sender == 0) _ }
}
// ----
// ParserError: (52-53): Expected ';' but got '}'
