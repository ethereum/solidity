{
	let x:bool
	for {let i := 0} x {i := add(i, 1)}
	{
		break
	}
}
// ====
// dialect: evmTyped
// ----
