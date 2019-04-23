contract A {
	fixed40x40 pi = 3.14.15;
}
// The second error message below is a result of
// expectToken() tacitly inserting a ';' before '.15'.
// .15 cannot start a statement. Adding the LHS of what was
// being worked on, i.e. Statement may help.
// ----
// ParserError: (34-37): Expected ';' but got 'Number'
// ParserError: (34-37): Function, variable, struct or modifier declaration expected.
