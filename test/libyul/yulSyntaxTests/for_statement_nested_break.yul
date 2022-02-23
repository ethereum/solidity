{
	let x:bool
	for {let i := 0} x {}
	{
		function f() { break }
	}
}
// ====
// dialect: evmTyped
// ----
// SyntaxError 2592: (57-62): Keyword "break" needs to be inside a for-loop body.
