{
	let x:u256
	function f() -> x:u256 {}
}
// ----
// DeclarationError 1395: (15-40): Variable name x already taken in this scope.
