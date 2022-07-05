contract c {
	enum foo { WARNING, 1, }
}
// ----
// ParserError 1612: (34-35): Expected identifier after ','