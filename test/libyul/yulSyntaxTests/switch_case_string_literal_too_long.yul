{
	let x:u256
	switch x
	case "012345678901234567890123456789012":u256 {}
}
// ====
// dialect: evmTyped
// ----
// TypeError 3069: (30-70): String literal too long (33 > 32)
