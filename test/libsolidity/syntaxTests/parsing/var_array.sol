contract Foo {
	function f() { var[] a; }
}
// ----
// ParserError: (34-34): Expected token Identifier got 'LBrack'
