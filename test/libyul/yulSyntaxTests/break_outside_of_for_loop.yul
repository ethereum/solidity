{
	let x:bool
	if x { break }
}
// ====
// dialect: evmTyped
// ----
// SyntaxError 2592: (22-27='break'): Keyword "break" needs to be inside a for-loop body.
