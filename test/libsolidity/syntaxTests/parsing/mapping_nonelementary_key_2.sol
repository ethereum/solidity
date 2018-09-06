contract c {
	struct S {
		uint x;
	}
	mapping(S => uint) data;
}
// ----
// ParserError: (47-48): Expected elementary type name for mapping key type
