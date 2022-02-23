{
	let x:bool
	for {let i := 0 break} x {i := add(i, 1)} {}
}
// ====
// dialect: evmTyped
// ----
// SyntaxError 9615: (31-36): Keyword "break" in for-loop init block is not allowed.
