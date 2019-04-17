contract c {
	modifier mod { if (msg.sender == 0) _ }
}
// ----
// ParserError: (52-53): Expected ';' but got '}'.
// ParserError: (56-56): Can not find token to synchronize to.
