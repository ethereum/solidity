{
	switch 0:u256
	case 0:u256 {}
	case "":u256 {}
}
// ====
// dialect: evmTyped
// ----
// DeclarationError 6792: (34-49='case "":u256 {}'): Duplicate case "0" defined.
