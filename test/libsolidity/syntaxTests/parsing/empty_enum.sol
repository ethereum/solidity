contract c {
	enum foo { }
}
// ----
// ParserError: (25-25): enum with no members is not allowed.
