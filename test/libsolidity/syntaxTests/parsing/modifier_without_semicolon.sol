contract c {
	modifier mod { if (msg.sender == 0) _ }
}
// ----
// ParserError: (52-52): Expected ';' but got '}'
