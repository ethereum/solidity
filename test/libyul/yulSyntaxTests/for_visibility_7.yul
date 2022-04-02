{
	for {} i {} { let i := 1 }
}
// ====
// dialect: evm
// ----
// DeclarationError 8198: (10-11='i'): Identifier "i" not found.
