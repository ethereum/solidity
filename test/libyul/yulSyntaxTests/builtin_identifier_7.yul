{
	function f() -> a, b {}
	add, mul := f()
}
// ----
// ParserError 6272: (31-32): Cannot assign to builtin function "add".
