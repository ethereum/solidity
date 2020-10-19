{
	for { pop(i) } 1 { } { let i := 1 }
}
// ====
// dialect: evm
// ----
// DeclarationError 8198: (13-14): Identifier not found.
