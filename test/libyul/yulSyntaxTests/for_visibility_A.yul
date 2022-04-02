{
	for { let x := 1 } 1 {} { let x := 1 }
}
// ====
// dialect: yul
// ----
// DeclarationError 1395: (29-39='let x := 1'): Variable name x already taken in this scope.
