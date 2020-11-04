{
	for {} 1 { pop(i) } { let i := 1 }
}
// ====
// dialect: evm
// ----
// DeclarationError 8198: (18-19): Identifier not found.
