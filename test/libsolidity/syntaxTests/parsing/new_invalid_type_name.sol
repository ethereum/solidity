contract C {
	function f() {
		new var;
	}
}
// ----
// ParserError: (35-38): Expected explicit type name.
