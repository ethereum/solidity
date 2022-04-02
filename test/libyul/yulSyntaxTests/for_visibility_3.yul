{
	for {} 1 { let i := 1 } { pop(i) }
}
// ====
// dialect: evm
// ----
// DeclarationError 8198: (33-34='i'): Identifier "i" not found.
