contract C {
	event A();
	function f() {
		emit A;
	}
}
// ----
// ParserError: (49-49): Expected '(' but got ';'
