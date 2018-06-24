contract Foo {
	function f() { var[] a; }
}
// ----
// ParserError: (34-35): Expected identifier but got '['
