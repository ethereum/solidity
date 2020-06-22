contract Foo {
	function f() { var[] a; }
}
// ----
// ParserError 2314: (34-35): Expected identifier but got '['
