{
	let x:u256
	function f() -> a:u256, b:u256 {}
	123:u256, x := f()
}
// ====
// dialect: evmTyped
// ----
// ParserError 2856: (58-59): Variable name must precede "," in multiple assignment.
