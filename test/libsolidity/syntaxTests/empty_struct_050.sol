pragma experimental "v0.5.0";
contract test {
	struct A {}
}
// ----
// SyntaxError: Defining empty structs is disallowed.
