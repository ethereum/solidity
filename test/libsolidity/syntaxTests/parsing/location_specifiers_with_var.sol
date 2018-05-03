contract Foo {
	function f() { var memory x; }
}
// ----
// ParserError: (35-35): Location specifier needs explicit type name.
