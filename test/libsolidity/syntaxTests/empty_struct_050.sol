pragma experimental "v0.5.0";
contract test {
	struct A {}
}
// ----
// SyntaxError: (47-58): Defining empty structs is disallowed.
