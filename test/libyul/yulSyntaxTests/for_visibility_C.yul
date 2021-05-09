{
	let x := 1 for {} 1 { let x := 1 } {}
}
// ----
// DeclarationError 1395: (25-35): Variable name x already taken in this scope.
