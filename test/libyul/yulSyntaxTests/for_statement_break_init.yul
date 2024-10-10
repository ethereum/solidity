{
	let x
	for {let i := 0 break} x {i := add(i, 1)} {}
}
// ----
// SyntaxError 9615: (26-31): Keyword "break" in for-loop init block is not allowed.
