contract test {
	struct A {}
}
// ----
// SyntaxError 5306: (17-28='struct A {}'): Defining empty structs is disallowed.
