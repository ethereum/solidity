contract Foo {
	function f() { var memory x; }
}
// ----
// ParserError 6933: (31-34): Expected primary expression.
