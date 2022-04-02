{
	let x := 1 for { let x := 1 } 1 {} {}
}
// ----
// DeclarationError 1395: (20-30='let x := 1'): Variable name x already taken in this scope.
