contract c {
	enum foo { }
}
// ----
// ParserError: (25-26): enum with no members is not allowed.
