{
	function f() -> x, y {}
	let x, x := f()
}
// ----
// DeclarationError 1395: (28-43): Variable name x already taken in this scope.
