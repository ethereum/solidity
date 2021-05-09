{
	let x:u256
	function f() -> a:u256, b:u256 {}
	x, 123:u256 := f()
}
// ====
// dialect: evmTyped
// ----
// ParserError 2856: (62-64): Variable name must precede ":=" in assignment.
