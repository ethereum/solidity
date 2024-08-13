{
	let x
	function f() -> a, b {}
	123, x := f()
}
// ----
// ParserError 2856: (38-39): Variable name must precede "," in multiple assignment.
