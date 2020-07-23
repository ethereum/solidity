contract C {
	function f() {
		new var;
	}
}
// ----
// ParserError 3546: (35-38): Expected type name
