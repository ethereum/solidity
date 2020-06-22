contract Foo {
	function f() { var memory x; }
}
// ----
// ParserError 7439: (35-41): Location specifier needs explicit type name.
