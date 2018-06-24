contract Foo {
	function f() { var memory x; }
}
// ----
// ParserError: (35-41): Location specifier needs explicit type name.
