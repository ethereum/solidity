{
	// Check that body and post are not sub-scopes of each other.
	for {} 1 { let x := 1 } { let x := 1 }
}
// ====
// dialect: evm
