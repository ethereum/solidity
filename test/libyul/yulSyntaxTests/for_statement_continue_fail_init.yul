{
	let x
	for {let i := 0 continue} x {i := add(i, 1)}
	{
	}
}
// ----
// SyntaxError 9615: (26-34): Keyword "continue" in for-loop init block is not allowed.
