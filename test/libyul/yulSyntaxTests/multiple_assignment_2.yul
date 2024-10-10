{
	let x
	function f() -> a, b {}
	x, 123 := f()
}
// ----
// ParserError 2856: (42-44): Variable name must precede ":=" in assignment.
