{
	function f() -> x, y {}
	let x, x := f()
}
// ----
// DeclarationError 1395: (28-43='let x, x := f()'): Variable name x already taken in this scope.
