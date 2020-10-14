{
	let x:bool
	for {let i := 0} x {i := add(i, 1)}
	{
		continue
	}
}
// ====
// dialect: evmTyped
// ----
