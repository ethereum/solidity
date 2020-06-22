contract c {
	enum foo { }
}
// ----
// ParserError 3147: (25-26): enum with no members is not allowed.
