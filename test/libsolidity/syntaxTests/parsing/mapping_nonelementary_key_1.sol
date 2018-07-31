contract c {
	mapping(uint[] => uint) data;
}
// ----
// ParserError: (26-27): Expected '=>' but got '['
