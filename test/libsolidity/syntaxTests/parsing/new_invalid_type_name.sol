contract C {
	function f() {
		new var;
	}
}
// ----
// ParserError 7059: (35-38): Expected explicit type name.
