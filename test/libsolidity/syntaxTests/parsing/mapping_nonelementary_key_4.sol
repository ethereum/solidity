contract c {
	mapping(string[] => uint) data;
}
// ----
// ParserError: (28-29): Expected '=>' but got '['
// ParserError: (28-29): Expected type name
