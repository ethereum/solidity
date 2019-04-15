contract c {
	modifier mod { if (msg.sender == 0) _ }
}
// ----
// ParserError: (52-53): Expected ';' but got '}'.
// ParserError: (53-56): Can not find token to synchronize to.
