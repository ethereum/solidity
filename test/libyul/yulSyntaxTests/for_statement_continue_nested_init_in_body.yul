{
	for {} 1 {}
	{
		let x
		for { continue } x {} {}
	}
}
// ----
// SyntaxError 9615: (34-42): Keyword "continue" in for-loop init block is not allowed.
