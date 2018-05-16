contract C {
	event A();
	function f() {
		emit A;
	}
}
// ----
// ParserError: (49-50): Expected '(' but got ';'
