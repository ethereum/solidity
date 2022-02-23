{
	let x:bool
	for {let i := 0 continue} x {i := add(i, 1)}
	{
	}
}
// ====
// dialect: evmTyped
// ----
// SyntaxError 9615: (31-39): Keyword "continue" in for-loop init block is not allowed.
