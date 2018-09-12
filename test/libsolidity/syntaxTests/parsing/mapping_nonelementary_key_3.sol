contract c {
	struct S {
		string s;
	}
	mapping(S => uint) data;
}
// ----
// ParserError: (49-50): Expected elementary type name for mapping key type
