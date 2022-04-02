{
	{ let g := 0 }
	function g() {}
}
// ----
// DeclarationError 1395: (5-15='let g := 0'): Variable name g already taken in this scope.
