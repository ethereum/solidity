{
	let x:bool
	for {let i := 0 } x {i := add(i, 1) break} {}
}
// ====
// dialect: evmTyped
// ----
// SyntaxError 2461: (51-56): Keyword "break" in for-loop post block is not allowed.
