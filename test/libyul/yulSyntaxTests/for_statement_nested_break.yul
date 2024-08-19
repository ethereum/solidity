{
	let x
	for {let i := 0} x {}
	{
		function f() { break }
	}
}
// ----
// SyntaxError 2592: (52-57): Keyword "break" needs to be inside a for-loop body.
