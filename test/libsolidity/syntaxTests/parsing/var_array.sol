contract Foo {
	function f() { var[] a; }
}
// ----
// ParserError 6933: (31-34): Expected primary expression.
