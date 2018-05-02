contract C {
	function f() {
		new var;
	}
}
// ----
// ParserError: (35-35): Expected explicit type name.
