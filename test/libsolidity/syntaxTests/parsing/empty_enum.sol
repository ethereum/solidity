contract c {
	enum foo { }
}
// ----
// ParserError 3147: (25-26): Enum with no members is not allowed.
