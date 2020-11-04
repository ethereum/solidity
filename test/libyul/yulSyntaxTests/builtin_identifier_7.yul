{
	// Test for the unreachable 6272_error
	function f() -> a, b {}
	add, mul := f()
}
// ----
// ParserError 2314: (71-72): Expected '(' but got ','
