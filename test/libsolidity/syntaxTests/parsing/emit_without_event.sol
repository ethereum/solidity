contract C {
	event A();
	function f() {
		emit A;
	}
}
// ----
// ParserError 2314: (49-50): Expected '(' but got ';'
