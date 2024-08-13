{
	let x
	for {let i := 0 } x {i := add(i, 1) break} {}
}
// ----
// SyntaxError 2461: (46-51): Keyword "break" in for-loop post block is not allowed.
