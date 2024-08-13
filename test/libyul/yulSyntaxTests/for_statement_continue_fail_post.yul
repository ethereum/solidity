{
	let x
	for {let i := 0} x {i := add(i, 1) continue} {}
}
// ----
// SyntaxError 2461: (45-53): Keyword "continue" in for-loop post block is not allowed.
