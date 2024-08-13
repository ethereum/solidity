{
	let x
	function f() -> x {}
}
// ----
// DeclarationError 1395: (10-30): Variable name x already taken in this scope.
