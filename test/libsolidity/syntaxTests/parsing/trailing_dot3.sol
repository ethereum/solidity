contract test {
	uint a = 2.
// ----
// ParserError: (29-29): Expected identifier but got end of source
// ParserError: (29-29): Expected ';' but got end of source
// ParserError: (29-29): Function, variable, struct or modifier declaration expected.
// ParserError: (29-29): In <ContractDefinition>, '}' is expected; got end of source instead.
